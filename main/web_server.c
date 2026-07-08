#include "web_server.h"

#include <stdio.h>
#include <string.h>

#include "esp_event.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "lwip/inet.h"
#include "nvs_flash.h"

#define WIFI_SSID "SH!P"
#define WIFI_PASSWORD "123456789"
#define WIFI_CHANNEL 1
#define WIFI_MAX_CONN 4

static const char *TAG = "web_server";
static system_state_t *s_state;

static const char INDEX_HTML[] =
    "<!DOCTYPE html><html><head><meta charset=\"UTF-8\"><title>船舶智能监测系统</title>"
    "<style>body{font-family:Arial,'Microsoft YaHei',sans-serif;background:#0f172a;color:#e5e7eb;padding:20px}"
    "h1{color:#38bdf8}.card{background:#1e293b;border-radius:12px;padding:16px;margin-bottom:12px;box-shadow:0 4px 12px rgba(0,0,0,.3)}"
    ".label{color:#94a3b8}.value{font-size:22px;font-weight:bold;color:#f8fafc}.alarm{color:#f87171;font-size:26px;font-weight:bold}.normal{color:#4ade80;font-size:26px;font-weight:bold}"
    "button{background:#2563eb;color:white;border:0;border-radius:8px;padding:10px 14px;margin:4px;font-size:16px}button.warn{background:#dc2626}button.ok{background:#16a34a}#cmdResult{color:#facc15;margin-top:8px}</style></head>"
    "<body><h1>船舶智能物联网监测系统</h1>"
    "<div class=\"card\"><div class=\"label\">当前报警状态</div><div id=\"alarm\" class=\"normal\">系统正常</div></div>"
    "<div class=\"card\"><div class=\"label\">网页控制</div><button onclick=\"sendCmd('STATUS')\">刷新状态</button><button class=\"warn\" onclick=\"sendCmd('ALARM_OFF')\">关闭声光报警</button><button class=\"ok\" onclick=\"sendCmd('ALARM_ON')\">恢复声光报警</button><div id=\"cmdResult\"></div></div>"
    "<div class=\"card\"><div class=\"label\">温湿度</div><div class=\"value\">温度：<span id=\"temp\">--</span> ℃　湿度：<span id=\"humi\">--</span> %</div></div>"
    "<div class=\"card\"><div class=\"label\">气体浓度</div><div class=\"value\">MQ-2：<span id=\"gas\">--</span></div></div>"
    "<div class=\"card\"><div class=\"label\">安全检测</div><div class=\"value\">火焰：<span id=\"flame\">--</span><br>人体：<span id=\"pir\">--</span><br>障碍物：<span id=\"obstacle\">--</span></div></div>"
    "<div class=\"card\"><div class=\"label\">船体姿态</div><div class=\"value\">X倾角：<span id=\"angleX\">--</span> °<br>Y倾角：<span id=\"angleY\">--</span> °</div></div>"
    "<script>function updateData(){fetch('/data').then(r=>r.json()).then(data=>{temp.innerText=data.temp;humi.innerText=data.humi;gas.innerText=data.gas;flame.innerText=data.flame;pir.innerText=data.pir;obstacle.innerText=data.obstacle;angleX.innerText=data.angleX;angleY.innerText=data.angleY;alarm.innerText=data.alarm;alarm.className=data.triggered==1?'alarm':'normal'}).catch(()=>{alarm.innerText='通信离线，正在重连...';alarm.className='alarm'})}"
    "function sendCmd(name){fetch('/cmd?name='+encodeURIComponent(name)).then(r=>r.text()).then(t=>{cmdResult.innerText=t;updateData()}).catch(()=>cmdResult.innerText='命令发送失败')}setInterval(updateData,1000);updateData();</script></body></html>";

static esp_err_t handle_root(httpd_req_t *req)
{
    httpd_resp_set_type(req, "text/html; charset=utf-8");
    return httpd_resp_send(req, INDEX_HTML, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t handle_data(httpd_req_t *req)
{
    char json[512];
    system_state_update_alarm(s_state);
    system_state_build_json(s_state, json, sizeof(json));
    httpd_resp_set_type(req, "application/json; charset=utf-8");
    return httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);
}

static esp_err_t handle_cmd(httpd_req_t *req)
{
    char query[128] = {0};
    char command[32] = {0};
    char response[256];

    if (httpd_req_get_url_query_str(req, query, sizeof(query)) != ESP_OK ||
        httpd_query_key_value(query, "name", command, sizeof(command)) != ESP_OK) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "缺少命令参数 name");
        return ESP_FAIL;
    }

    system_state_handle_command(s_state, command, response, sizeof(response));
    ESP_LOGI(TAG, "Web command: %s -> %s", command, response);
    httpd_resp_set_type(req, "text/plain; charset=utf-8");
    return httpd_resp_send(req, response, HTTPD_RESP_USE_STRLEN);
}

static void wifi_ap_start(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    wifi_config_t wifi_config = {0};
    strncpy((char *)wifi_config.ap.ssid, WIFI_SSID, sizeof(wifi_config.ap.ssid));
    strncpy((char *)wifi_config.ap.password, WIFI_PASSWORD, sizeof(wifi_config.ap.password));
    wifi_config.ap.ssid_len = strlen(WIFI_SSID);
    wifi_config.ap.channel = WIFI_CHANNEL;
    wifi_config.ap.max_connection = WIFI_MAX_CONN;
    wifi_config.ap.authmode = WIFI_AUTH_WPA_WPA2_PSK;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Wi-Fi 热点已启动");
    ESP_LOGI(TAG, "Wi-Fi 名称: %s", WIFI_SSID);
    ESP_LOGI(TAG, "Wi-Fi 密码: %s", WIFI_PASSWORD);
    ESP_LOGI(TAG, "手机访问地址: http://192.168.4.1/");
}

void web_server_start(system_state_t *state)
{
    s_state = state;
    wifi_ap_start();

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;
    ESP_ERROR_CHECK(httpd_start(&server, &config));

    httpd_uri_t root_uri = {.uri = "/", .method = HTTP_GET, .handler = handle_root};
    httpd_uri_t data_uri = {.uri = "/data", .method = HTTP_GET, .handler = handle_data};
    httpd_uri_t cmd_uri = {.uri = "/cmd", .method = HTTP_GET, .handler = handle_cmd};
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &root_uri));
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &data_uri));
    ESP_ERROR_CHECK(httpd_register_uri_handler(server, &cmd_uri));
    ESP_LOGI(TAG, "Web服务器已启动");
}



