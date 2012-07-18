/* RX filter BW = 100.000000 */
/* Address config = No address check */
/* Packet length = 255 */
/* Symbol rate = 38.3606 */
/* PA ramping = false */
/* Carrier frequency = 434.549988 */
/* Bit rate = 38.3606 */
/* Whitening = true */
/* Manchester enable = false */
/* Modulation format = 2-GFSK */
/* Packet length mode = Variable */
/* Device address = 0 */
/* TX power = 15 */
/* Deviation = 20.507812 */
/***************************************************************
 *  SmartRF Studio(tm) Export
 *
 *  Radio register settings specifed with address, value
 *
 *  RF device: CC1120
 *
 ***************************************************************/

        CC1120_SYNC3,                          0xD3,       /* Sync Word Configuration [31:24] */
        CC1120_SYNC2,                          0x91,       /* Sync Word Configuration [23:16] */
        CC1120_SYNC1,                          0xD3,       /* Sync Word Configuration [15:8] */
        CC1120_SYNC0,                          0x91,       /* Sync Word Configuration [7:0] */

        CC1120_SYNC_CFG1,				   /* Sync Word Detection Configuration */
		(CC1120_SYNC_CFG1_DEM_CFG_PQT_GATING_ENABLED << CC1120_SYNC_CFG1_DEM_CFG) |
		(0x07 << CC1120_SYNC_CFG1_SYNC_THR),
        CC1120_SYNC_CFG0,
		(CC1120_SYNC_CFG0_SYNC_MODE_16_BITS << CC1120_SYNC_CFG0_SYNC_MODE) |
		(CC1120_SYNC_CFG0_SYNC_NUM_ERROR_2 << CC1120_SYNC_CFG0_SYNC_NUM_ERROR),
        CC1120_DCFILT_CFG,                     0x1c,       /* Digital DC Removal Configuration */
        CC1120_PREAMBLE_CFG1,                         	   /* Preamble Length Configuration */
		(CC1120_PREAMBLE_CFG1_NUM_PREAMBLE_4_BYTES << CC1120_PREAMBLE_CFG1_NUM_PREAMBLE) |
		(CC1120_PREAMBLE_CFG1_PREAMBLE_WORD_AA << CC1120_PREAMBLE_CFG1_PREAMBLE_WORD),
        CC1120_PREAMBLE_CFG0,
		(1 << CC1120_PREAMBLE_CFG0_PQT_EN) |
		(0x6 << CC1120_PREAMBLE_CFG0_PQT),
        CC1120_FREQ_IF_CFG,                    0x40,       /* RX Mixer Frequency Configuration */
        CC1120_IQIC,                           0x46,       /* Digital Image Channel Compensation Configuration */
        CC1120_CHAN_BW,                        0x02,       /* Channel Filter Configuration */

        CC1120_MDMCFG1,                     		   /* General Modem Parameter Configuration */
		(0 << CC1120_MDMCFG1_CARRIER_SENSE_GATE) |
		(1 << CC1120_MDMCFG1_FIFO_EN) |
		(0 << CC1120_MDMCFG1_MANCHESTER_EN) |
		(0 << CC1120_MDMCFG1_INVERT_DATA_EN) |
		(0 << CC1120_MDMCFG1_COLLISION_DETECT_EN) |
		(CC1120_MDMCFG1_DVGA_GAIN_9 << CC1120_MDMCFG1_DVGA_GAIN) |
		(0 << CC1120_MDMCFG1_SINGLE_ADC_EN),
        CC1120_MDMCFG0,                        0x05,       /* General Modem Parameter Configuration */

        CC1120_AGC_REF,                        0x20,       /* AGC Reference Level Configuration */
        CC1120_AGC_CS_THR,                     0x19,       /* Carrier Sense Threshold Configuration */
        CC1120_AGC_GAIN_ADJUST,                0x00,       /* RSSI Offset Configuration */
        CC1120_AGC_CFG3,                       0x91,       /* AGC Configuration */
        CC1120_AGC_CFG2,                       0x20,       /* AGC Configuration */
        CC1120_AGC_CFG1,                       0xa9,       /* AGC Configuration */
        CC1120_AGC_CFG0,                       0xcf,       /* AGC Configuration */
        CC1120_FIFO_CFG,		       		   /* FIFO Configuration */
		(0 << CC1120_FIFO_CFG_CRC_AUTOFLUSH) |
		(0x40 << CC1120_FIFO_CFG_FIFO_THR),
        CC1120_DEV_ADDR,                       0x00,       /* Device Address Configuration */
        CC1120_SETTLING_CFG,                          	   /* Frequency Synthesizer Calibration and Settling Configuration */
		(CC1120_SETTLING_CFG_FS_AUTOCAL_IDLE_TO_ON << CC1120_SETTLING_CFG_FS_AUTOCAL) |
		(CC1120_SETTLING_CFG_LOCK_TIME_50_20 << CC1120_SETTLING_CFG_LOCK_TIME) |
		(CC1120_SETTLING_CFG_FSREG_TIME_60 << CC1120_SETTLING_CFG_FSREG_TIME),
        CC1120_FS_CFG,                                	   /* Frequency Synthesizer Configuration */
		(1 << CC1120_FS_CFG_LOCK_EN) |
		(CC1120_FS_CFG_FSD_BANDSELECT_410_480 << CC1120_FS_CFG_FSD_BANDSELECT),
        CC1120_WOR_CFG1,                       0x08,       /* eWOR Configuration, Reg 1 */
        CC1120_WOR_CFG0,                       0x21,       /* eWOR Configuration, Reg 0 */
        CC1120_WOR_EVENT0_MSB,                 0x00,       /* Event 0 Configuration */
        CC1120_WOR_EVENT0_LSB,                 0x00,       /* Event 0 Configuration */
#if 0
        CC1120_PKT_CFG2,                       0x04,       /* Packet Configuration, Reg 2 */
        CC1120_PKT_CFG1,                       0x45,       /* Packet Configuration, Reg 1 */
        CC1120_PKT_CFG0,                       0x00,       /* Packet Configuration, Reg 0 */
#endif
        CC1120_RFEND_CFG1,                     0x0f,       /* RFEND Configuration, Reg 1 */
        CC1120_RFEND_CFG0,                     0x00,       /* RFEND Configuration, Reg 0 */
	//        CC1120_PA_CFG2,                        0x3f,       /* Power Amplifier Configuration, Reg 2 */
	CC1120_PA_CFG2,                        0x3f,       /* Power Amplifier Configuration, Reg 2 */
        CC1120_PA_CFG1,                        0x56,       /* Power Amplifier Configuration, Reg 1 */
        CC1120_PA_CFG0,                        0x7b,       /* Power Amplifier Configuration, Reg 0 */
        CC1120_PKT_LEN,                        0xff,       /* Packet Length Configuration */
        CC1120_IF_MIX_CFG,                     0x00,       /* IF Mix Configuration */
        CC1120_FREQOFF_CFG,                    0x22,       /* Frequency Offset Correction Configuration */
        CC1120_TOC_CFG,                        0x0b,       /* Timing Offset Correction Configuration */
        CC1120_MARC_SPARE,                     0x00,       /* MARC Spare */
        CC1120_ECG_CFG,                        0x00,       /* External Clock Frequency Configuration */
        CC1120_SOFT_TX_DATA_CFG,               0x00,       /* Soft TX Data Configuration */
        CC1120_EXT_CTRL,                       0x00,       /* External Control Configuration */
        CC1120_RCCAL_FINE,                     0x00,       /* RC Oscillator Calibration (fine) */
        CC1120_RCCAL_COARSE,                   0x00,       /* RC Oscillator Calibration (coarse) */
        CC1120_RCCAL_OFFSET,                   0x00,       /* RC Oscillator Calibration Clock Offset */
        CC1120_FREQOFF1,                       0x00,       /* Frequency Offset (MSB) */
        CC1120_FREQOFF0,                       0x00,       /* Frequency Offset (LSB) */
        CC1120_IF_ADC2,                        0x02,       /* Analog to Digital Converter Configuration, Reg 2 */
        CC1120_IF_ADC1,                        0xa6,       /* Analog to Digital Converter Configuration, Reg 1 */
        CC1120_IF_ADC0,                        0x04,       /* Analog to Digital Converter Configuration, Reg 0 */
        CC1120_FS_DIG1,                        0x00,       /*  */
        CC1120_FS_DIG0,                        0x5f,       /*  */
        CC1120_FS_CAL3,                        0x00,       /*  */
        CC1120_FS_CAL2,                        0x20,       /*  */
        CC1120_FS_CAL1,                        0x40,       /*  */
        CC1120_FS_CAL0,                        0x0e,       /*  */
        CC1120_FS_CHP,                         0x28,       /* Charge Pump Configuration */
        CC1120_FS_DIVTWO,                      0x03,       /* Divide by 2 */
        CC1120_FS_DSM1,                        0x00,       /* Digital Synthesizer Module Configuration, Reg 1 */
        CC1120_FS_DSM0,                        0x33,       /* Digital Synthesizer Module Configuration, Reg 0 */
        CC1120_FS_DVC1,                        0xff,       /* Divider Chain Configuration, Reg 1 */
        CC1120_FS_DVC0,                        0x17,       /* Divider Chain Configuration, Reg 0 */
        CC1120_FS_LBI,                         0x00,       /* Local Bias Configuration */
        CC1120_FS_PFD,                         0x50,       /* Phase Frequency Detector Configuration */
        CC1120_FS_PRE,                         0x6e,       /* Prescaler Configuration */
        CC1120_FS_REG_DIV_CML,                 0x14,       /*  */
        CC1120_FS_SPARE,                       0xac,       /*  */
        CC1120_FS_VCO4,                        0x14,       /* VCO Configuration, Reg 4 */
        CC1120_FS_VCO3,                        0x00,       /* VCO Configuration, Reg 3 */
        CC1120_FS_VCO2,                        0x00,       /* VCO Configuration, Reg 2 */
        CC1120_FS_VCO1,                        0x00,       /* VCO Configuration, Reg 1 */
        CC1120_FS_VCO0,                        0xb4,       /* VCO Configuration, Reg 0 */
        CC1120_GBIAS6,                         0x00,       /* Global Bias Configuration, Reg 6 */
        CC1120_GBIAS5,                         0x02,       /* Global Bias Configuration, Reg 5 */
        CC1120_GBIAS4,                         0x00,       /* Global Bias Configuration, Reg 4 */
        CC1120_GBIAS3,                         0x00,       /* Global Bias Configuration, Reg 3 */
        CC1120_GBIAS2,                         0x10,       /* Global Bias Configuration, Reg 2 */
        CC1120_GBIAS1,                         0x00,       /* Global Bias Configuration, Reg 1 */
        CC1120_GBIAS0,                         0x00,       /* Global Bias Configuration, Reg 0 */
        CC1120_IFAMP,                          0x01,       /* Intermediate Frequency Amplifier Configuration */
        CC1120_LNA,                            0x01,       /* Low Noise Amplifier Configuration */
        CC1120_RXMIX,                          0x01,       /* RX Mixer Configuration */
        CC1120_XOSC5,                          0x0e,       /* Crystal Oscillator Configuration, Reg 5 */
        CC1120_XOSC4,                          0xa0,       /* Crystal Oscillator Configuration, Reg 4 */
        CC1120_XOSC3,                          0x03,       /* Crystal Oscillator Configuration, Reg 3 */
        CC1120_XOSC2,                          0x04,       /* Crystal Oscillator Configuration, Reg 2 */
        CC1120_XOSC1,                          0x01,       /* Crystal Oscillator Configuration, Reg 1 */
        CC1120_XOSC0,                          0x00,       /* Crystal Oscillator Configuration, Reg 0 */
        CC1120_ANALOG_SPARE,                   0x00,       /*  */
        CC1120_PA_CFG3,                        0x00,       /* Power Amplifier Configuration, Reg 3 */
        CC1120_WOR_TIME1,                      0x00,       /* eWOR Timer Status (MSB) */
        CC1120_WOR_TIME0,                      0x00,       /* eWOR Timer Status (LSB) */
        CC1120_WOR_CAPTURE1,                   0x00,       /* eWOR Timer Capture (MSB) */
        CC1120_WOR_CAPTURE0,                   0x00,       /* eWOR Timer Capture (LSB) */
        CC1120_BIST,                           0x00,       /* MARC BIST */
        CC1120_DCFILTOFFSET_I1,                0x00,       /* DC Filter Offset I (MSB) */
        CC1120_DCFILTOFFSET_I0,                0x00,       /* DC Filter Offset I (LSB) */
        CC1120_DCFILTOFFSET_Q1,                0x00,       /* DC Filter Offset Q (MSB) */
        CC1120_DCFILTOFFSET_Q0,                0x00,       /* DC Filter Offset Q (LSB) */
        CC1120_IQIE_I1,                        0x00,       /* IQ Imbalance Value I (MSB) */
        CC1120_IQIE_I0,                        0x00,       /* IQ Imbalance Value I (LSB) */
        CC1120_IQIE_Q1,                        0x00,       /* IQ Imbalance Value Q (MSB) */
        CC1120_IQIE_Q0,                        0x00,       /* IQ Imbalance Value Q (LSB) */
        CC1120_RSSI1,                          0x80,       /* Received Signal Strength Indicator (MSB) */
        CC1120_RSSI0,                          0x00,       /* Received Signal Strength Indicator (LSB) */
        CC1120_MARCSTATE,                      0x41,       /* MARC State */
        CC1120_LQI_VAL,                        0x00,       /* Link Quality Indicator Value */
        CC1120_PQT_SYNC_ERR,                   0xff,       /* Preamble and Sync Word Error */
        CC1120_DEM_STATUS,                     0x00,       /* Demodulator Status */
        CC1120_FREQOFF_EST1,                   0x00,       /* Frequency Offset Estimate (MSB) */
        CC1120_FREQOFF_EST0,                   0x00,       /* Frequency Offset Estimate (LSB) */
        CC1120_AGC_GAIN3,                      0x00,       /* AGC Gain, Reg 3 */
        CC1120_AGC_GAIN2,                      0xd1,       /* AGC Gain, Reg 2 */
        CC1120_AGC_GAIN1,                      0x00,       /* AGC Gain, Reg 1 */
        CC1120_AGC_GAIN0,                      0x3f,       /* AGC Gain, Reg 0 */
        CC1120_SOFT_RX_DATA_OUT,               0x00,       /* Soft Decision Symbol Data */
        CC1120_SOFT_TX_DATA_IN,                0x00,       /* Soft TX Data Input Register */
        CC1120_ASK_SOFT_RX_DATA,               0x30,       /* AGC ASK Soft Decision Output */
        CC1120_RNDGEN,                         0x7f,       /* Random Number Value */
        CC1120_MAGN2,                          0x00,       /* Signal Magnitude after CORDIC [16] */
        CC1120_MAGN1,                          0x00,       /* Signal Magnitude after CORDIC [15:8] */
        CC1120_MAGN0,                          0x00,       /* Signal Magnitude after CORDIC [7:0] */
        CC1120_ANG1,                           0x00,       /* Signal Angular after CORDIC [9:8] */
        CC1120_ANG0,                           0x00,       /* Signal Angular after CORDIC [7:0] */
        CC1120_CHFILT_I2,                      0x08,       /* Channel Filter Data Real Part [18:16] */
        CC1120_CHFILT_I1,                      0x00,       /* Channel Filter Data Real Part [15:8] */
        CC1120_CHFILT_I0,                      0x00,       /* Channel Filter Data Real Part [7:0] */
        CC1120_CHFILT_Q2,                      0x00,       /* Channel Filter Data Imaginary Part [18:16] */
        CC1120_CHFILT_Q1,                      0x00,       /* Channel Filter Data Imaginary Part [15:8] */
        CC1120_CHFILT_Q0,                      0x00,       /* Channel Filter Data Imaginary Part [7:0] */
        CC1120_GPIO_STATUS,                    0x00,       /* GPIO Status */
        CC1120_FSCAL_CTRL,                     0x01,       /*  */
        CC1120_PHASE_ADJUST,                   0x00,       /*  */
        CC1120_PARTNUMBER,                     0x00,       /* Part Number */
        CC1120_PARTVERSION,                    0x00,       /* Part Revision */
        CC1120_SERIAL_STATUS,                  0x00,       /* Serial Status */
        CC1120_RX_STATUS,                      0x01,       /* RX Status */
        CC1120_TX_STATUS,                      0x00,       /* TX Status */
        CC1120_MARC_STATUS1,                   0x00,       /* MARC Status, Reg 1 */
        CC1120_MARC_STATUS0,                   0x00,       /* MARC Status, Reg 0 */
        CC1120_PA_IFAMP_TEST,                  0x00,       /*  */
        CC1120_FSRF_TEST,                      0x00,       /*  */
        CC1120_PRE_TEST,                       0x00,       /*  */
        CC1120_PRE_OVR,                        0x00,       /*  */
        CC1120_ADC_TEST,                       0x00,       /* ADC Test */
        CC1120_DVC_TEST,                       0x0b,       /* DVC Test */
        CC1120_ATEST,                          0x40,       /*  */
        CC1120_ATEST_LVDS,                     0x00,       /*  */
        CC1120_ATEST_MODE,                     0x00,       /*  */
        CC1120_XOSC_TEST1,                     0x3c,       /*  */
        CC1120_XOSC_TEST0,                     0x00,       /*  */
        CC1120_RXFIRST,                        0x00,       /* RX FIFO Pointer (first entry) */
        CC1120_TXFIRST,                        0x00,       /* TX FIFO Pointer (first entry) */
        CC1120_RXLAST,                         0x00,       /* RX FIFO Pointer (last entry) */
        CC1120_TXLAST,                         0x00,       /* TX FIFO Pointer (last entry) */

