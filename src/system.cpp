/*==============================================================================
   LoopSystem ESP32
  ============================================================================*/

//==============================================================================
//  Includes
//==============================================================================
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_freertos_hooks.h"

#include "system.h"
#include "SPIFFS.h"
#include "config.h"
#include "runtime_config.h"
#include "webserver.h"

#include "logger.h"

//==============================================================================
//  Defines
//==============================================================================
#define CMP_NAME        "System"

#define CPU_IDLE_SLEEP_TIME     (1)

//==============================================================================
//  Local types
//==============================================================================
typedef struct _systemCpuStats
{
    uint64_t    startTime;              // in uS
    uint64_t    idleTime;               // in uS
    uint64_t    totalIdleTime;          // in uS
    uint64_t    reportTimestamp;        // in uS
    uint8_t     currentLoad;            // in %
    uint8_t     minLoad;
    uint8_t     maxLoad;
} systemCpuStats;

//==============================================================================
//  Local function definitions
//==============================================================================

//==============================================================================
//  Local data
//==============================================================================
static systemCpuStats   cpuStats[portNUM_PROCESSORS];

String cpuUsage = String("");

//==============================================================================
//  Local functions
//==============================================================================
static void idleTask(void * arg)
{
    systemCpuStats * const stats = (systemCpuStats* const)arg;
    uint64_t start;
    stats->startTime = SystemGetTimeuS();
    while (1)
    {
        start = SystemGetTimeuS();
        SystemSleepMs(CPU_IDLE_SLEEP_TIME);
        stats->idleTime += SystemElapsedTimeuS(start);
    }
}

static void updateCpuStats(systemCpuStats * const stats)
{
    // Store now instead of using SystemElapsedTimeuS to avoid getting time twice
    uint64_t now = SystemGetTimeuS();
    uint64_t timeDelta = now - stats->reportTimestamp;

    stats->currentLoad = 100 - DIV_ROUND_CLOSEST((stats->idleTime * 100), timeDelta);
    stats->totalIdleTime += stats->idleTime;
    Log(eLogDebug, CMP_NAME, "updateCpuStats: idleTime: %" PRIu64 ", timeDelta: %" PRIu64 ", calculated:%d", stats->idleTime, timeDelta, stats->currentLoad);

    if (SystemElapsedTimeuS(stats->startTime) > 1000000)
    {
        if (stats->currentLoad > stats->maxLoad)
        {
            stats->maxLoad = stats->currentLoad;
        }

        if (stats->currentLoad < stats->minLoad)
        {
            stats->minLoad = stats->currentLoad;
        }
    }

    stats->idleTime = 0;
    stats->reportTimestamp = SystemGetTimeuS();
}

static void getCpuStats(systemCpuStats const * const stats, String & cpuStats)
{
    cpuStats += String(stats->currentLoad);
    cpuStats += String("/");
    cpuStats += String(stats->minLoad);
    cpuStats += String("/");
    cpuStats += String((uint8_t)(100 - DIV_ROUND_CLOSEST((stats->totalIdleTime * 100), SystemElapsedTimeuS(stats->startTime))));
    cpuStats += String("/");
    cpuStats += String(stats->maxLoad);
}

static void monitorTask(void *arg)
{
    while (1)
    {
        for (int i = 0; i < portNUM_PROCESSORS; i++)
        {
            updateCpuStats(&cpuStats[i]);
        }

        //SystemGetCpuUsage(cpuUsage);
        //Log(eLogDebug, CMP_NAME, "Cpu Usage: %s", cpuUsage.c_str());

        SystemSleepMs(SYSTEM_CPU_MONITOR_PERIOD);
    }
}

static void initCpuCoreMonitor(uint8_t coreId)
{
    BaseType_t retVal;
    cpuStats[coreId].minLoad = 100;
    retVal = xTaskCreatePinnedToCore(idleTask, "CoreIdle", 1024, &cpuStats[coreId], 0, NULL, coreId);
    if (pdPASS == retVal)
    {
        Log(eLogInfo, CMP_NAME, "initCpuCoreMonitor: Initialized CPU monitor for core %d", coreId);
    }
    else
    {
        Log(eLogWarn, CMP_NAME, "initCpuCoreMonitor: Failed to initialize CPU monitor for core %d: error: %d", coreId, retVal);
    }
}

//==============================================================================
//  Exported functions
//==============================================================================
void SystemStartCpuStats()
{
    static esp_chip_info_t  chipInfo;
    esp_chip_info(&chipInfo);

    Log(eLogInfo, CMP_NAME, "SystemStartCpuStats: Got %d-core CPU, rev: %d, caps: 0x%04x",
        chipInfo.cores, chipInfo.revision, chipInfo.features);

    for (int i = 0; i < portNUM_PROCESSORS; i++)
    {
        initCpuCoreMonitor(i);
    }

    xTaskCreate(monitorTask, "CpuMonitor", 2048, NULL, 1, NULL);
}

void SystemRestart()
{
    Log(eLogWarn, CMP_NAME, "SystemRestart called!");
    WebserverCloseSockets();
    ConfigFinalize();
    SPIFFS.end();
    vTaskDelay(500 / portTICK_PERIOD_MS);
    ESP.restart();
}

void SystemGetMemoryInfo(String & memInfo)
{
    memInfo = String("Free: ");
    memInfo += String(esp_get_free_heap_size()/1024);
    memInfo += String("kB, Min Free: ");
    memInfo += String(esp_get_minimum_free_heap_size()/1024);
    memInfo += String("kB");
}

void SystemGetCpuUsage(String & cpuInfo)
{
    cpuInfo = String("Last/Min/Avg/Max: ");
    for (int i = 0; i < portNUM_PROCESSORS; i++)
    {
        cpuInfo += String("Core");
        cpuInfo += String(i);
        cpuInfo += String(": ");
        getCpuStats(&cpuStats[i], cpuInfo);
        if (i < portNUM_PROCESSORS - 1)
        {
            cpuInfo += String("; ");
        }
    }
}