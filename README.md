# Weighing_machine

## Description
This mechatronic device is a scale that incorporates a well-designed system to achieve accurate weight measurements. The core of this system is the YZC131C strain gauge sensor, which utilizes four strain gauges arranged in a bridge circuit. This sensor, built as a beam type, forms the foundation for precise weighing.

To ensure seamless signal transmission, two operational amplifiers are employed in this setup. The first amplifier utilizes the Non-Inverting Amplifier configuration, while the second one employs the Inverting Amplifier configuration. Together, they efficiently convert the sensor's output signal for transmission to the Analog-to-Digital Converter (ADC) of the stm32f302R8T6 microcontroller, which is the central component of this project.

To enhance the functionality of the microcontroller, the software aspect of the project integrates the freeRTOS operating system. This real-time operating system provides efficient task scheduling and resource management, ensuring optimal performance of the scale. Additionally, the project leverages the CMSIS DSP Software Library for advanced signal processing capabilities, enabling accurate data analysis and interpretation.

In summary, this mechatronic scale project utilizes the YZC131C strain gauge sensor, operational amplifiers, an stm32f302R8T6 microcontroller, freeRTOS operating system, and the CMSIS DSP Software Library. The combination of these components and software results in a reliable and precise weighing system capable of accurate signal processing and analysis.

## Circuit Diagram 


## Photo


## Block scheme