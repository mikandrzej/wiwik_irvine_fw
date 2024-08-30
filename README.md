To avoid BLE 0x103 error, you MUST comment out this section inside platformio binaries:
```.c
#ifdef CONFIG_BT_ENABLED
    if(!btInUse()){
        // esp_bt_controller_mem_release(ESP_BT_MODE_BTDM);
    }
#endif
```