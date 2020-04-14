<!--
#
# Licensed to the Apache Software Foundation (ASF) under one
# or more contributor license agreements.  See the NOTICE file
# distributed with this work for additional information
# regarding copyright ownership.  The ASF licenses this file
# to you under the Apache License, Version 2.0 (the
# "License"); you may not use this file except in compliance
# with the License.  You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing,
# software distributed under the License is distributed on an
# "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
# specific language governing permissions and limitations
# under the License.
#
-->

# BLE Temperature Sensor

## Overview

BLE Temperature Sensor is almost identical to the building instructions and code structure as the sample project.
https://github.com/lance-proxy/mynewt_proj

The difference includes new images and the following modifications to source files from the sample project. 

temp.c
-contains the ring buffer that temperature measurements get pushed on 

main.c
-added a new event/task to periodic send an event every 100ms to push temperature measurements to the ring buffer

gatt_svr.c
-empty ring buffer into GATT characteristic

bufferlog.png
-debug mode where each temperature measured gets printed when pushed to the ring buffer. This mode was used to verify the data being read is accurate


consolelogs.png
-debug mode off. The temperature measurements are shown when the GATT characteristic is read.

lightblue.png
-screenshot from lightblue showing the temperature measurements read.


## Notes

The way I approached the problem was to add a 100 ms periodic event to the event queue for reading the temperature sensor. Each event will push the temperature data to a ring buffer so we can maintain the ten most recent readings. Once that GATT characteristic is read, we empty the ring buffer into the GATT characteristic value.

This is my first time using newt, and I was actually impressed with its abstractions, was fun to use. I did have some concerns that could be worked on future improvements:

The first is the task priority of that temperature task. Should the priority be lower, the same, or higher than the BLE_LL job? I'm not quite sure if the priority matters since we are using an event queue design.

The second was the ring buffer. I noticed that there is a mbufs library, and I’m guessing that might be a better option for the ring buffer. It’s an improvement I would like to make but wanted to remain in the 2-3 hour time commitment. 

The third is the thread-safety of the ring buffer. This is something I would like to investigate further. As it stands, an interrupt could come in the middle of the ring buffer push operation. This could be problematic. 
