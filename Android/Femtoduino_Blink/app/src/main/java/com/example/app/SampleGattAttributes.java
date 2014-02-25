/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

package com.example.app;

import java.util.HashMap;

/**
 * This class includes a small subset of standard GATT attributes for demonstration purposes.
 */
public class SampleGattAttributes {
    private static HashMap<String, String> attributes = new HashMap();

    /**
     * NOTICE: UUIDs are CaSe SeNsItIvE!!!!
     */


    public static String HEART_RATE_MEASUREMENT = "00002a37-0000-1000-8000-00805f9b34fb";
    public static String CLIENT_CHARACTERISTIC_CONFIG = "00002902-0000-1000-8000-00805f9b34fb";
    public static String BLE_TX = "21819ab0-c937-4188-b0db-b9621e1696cd";
    public static String BLE_RX = "5fc569a0-74a9-4fa4-b8b7-8354c86e45a4";
    public static String BLE_SERVICE = "195ae58a-437a-489b-b0cd-b7c9c394bae4";


    static {

        // Sample Services.
        attributes.put("0000180d-0000-1000-8000-00805f9b34fb", "Heart Rate Service");
        attributes.put("0000180a-0000-1000-8000-00805f9b34fb", "Device Information Service");
        // Sample Characteristics.
        attributes.put(HEART_RATE_MEASUREMENT, "Heart Rate Measurement");
        attributes.put("00002a29-0000-1000-8000-00805f9b34fb", "Manufacturer Name String");

        attributes.put(BLE_SERVICE, "Data transceiver service");

        attributes.put(BLE_TX, "TX Data");
        attributes.put(BLE_RX, "RX Data");
//

        // Sample Services.
//        attributes.put("0000180d-0000-1000-8000-00805f9b34fb", "Heart Rate Service");
//        attributes.put("0000180a-0000-1000-8000-00805f9b34fb", "Device Information Service");

        // ...BLE113 GATT profile: data_transceiver
//        attributes.put("195AE58A-437A-489B-B0CD-B7C9C394BAE4", "Data Transceiver");

        // Sample Characteristics.
//        attributes.put(HEART_RATE_MEASUREMENT, "Heart Rate Measurement");
//        attributes.put("00002a29-0000-1000-8000-00805f9b34fb", "Manufacturer Name String");

        // ...BLE113 GATT profile: data_transceiver, RX data
//        attributes.put("5FC569A0-74A9-4FA4-B8B7-8354C86E45A4", "7"); // 0x01 - LED1, 0x02 - LED2, 0x04 - LED3
        // ...BLE
//        attributes.put("21819AB0-C937-4188-B0DB-B9621E1696CD", "TX Data");
    }

    public static String lookup(String uuid, String defaultName) {
        String name = attributes.get(uuid);
        return name == null ? defaultName : name;
    }
}