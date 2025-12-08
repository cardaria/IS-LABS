| I/O Device | Memory Range | Type | Interrupt # | Resource |
| :--- | :--- | :--- | :---: | :--- |
| **SDRAM (64 MB)** | `0x00000000` - `0x03ffffff` | Memory | | [Documentation]<br>*(Not needed for lab, but included for the curious reader)* |
| **Light Emitting Diodes (LEDs)** | `0x04000000` - `0x0400000f` | GPIO (Out) | | [Documentation]<br>*(Section 27.5 describes the programming model)* |
| **Toggle Switches** | `0x04000010` - `0x0400001f` | GPIO (In) | **17** | [Documentation]<br>*(Section 27.5 describes the programming model)* |
| **Timer (32-bit)** | `0x04000020` - `0x0400003f` | Device | **16** | [Documentation]<br>*(Section 23.4 describes the programming model)* |
| **JTAG UART** | `0x04000040` - `0x04000047` | Device | | [Documentation]<br>*(Not needed for lab, but included for the curious reader)* |
| **7 Segment Hex Display(s)** | `0x04000050` - `0x0400005f`<br>`0x04000060` - `0x0400006f`<br>`0x04000070` - `0x0400007f`<br>`0x04000080` - `0x0400008f`<br>`0x04000090` - `0x0400009f`<br>`0x040000a0` - `0x040000af` | GPIO (Out) | | [Documentation]<br>*(Section 27.5 describes the programming model)* |
| **Hardware Mutex** | `0x040000c0` - `0x040000c7` | Device | | [Documentation]<br>*(Not needed for lab, but could be used for projects)* |
| **Button** | `0x040000d0` - `0x040000df` | GPIO (In) | **18** | [Documentation]<br>*(Section 27.5 describes the programming model)* |
| **GPIO Expansion (2x20)** | `0x040000e0` - `0x040000ef` (Pins 0-31)<br>`0x040000f0` - `0x040000ff` (Pins 32-35) | GPIO (Bidirectional) | | [Documentation]<br>*(Section 27.5 describes the programming model)* |
| **VGA Pixel Buffer DMA**<br>(320 x 240 x 8-bit RGB color) | `0x04000100` - `0x0400010f` | Device | | [Documentation]<br>*(Section 4.10 describes the device)* |
| **VGA Screen Buffer**<br>(320x240x2) | `0x08000000` - `0x080257ff` | Memory | | [Documentation]<br>*(See lectures for more concrete examples of how to use VGA)* |