    #include <nrf24l01p.h>
    #include <SPI.h>

    enum
    {
      SPI_SS_PIN = 10,
      NRF_CE_PIN = 9,
     
      NRF_CHANNEL = 120,
      NRF_POWER_UP_DELAY = 2,
      NRF_PAYLOAD_LENGTH = 16
    };

    void setup()
    {
      SPI.begin();
       
      pinMode(SPI_SS_PIN, OUTPUT);
      digitalWrite(SPI_SS_PIN, HIGH);
     
      pinMode(NRF_CE_PIN, OUTPUT);
      digitalWrite(NRF_CE_PIN, HIGH);
     
      uint8_t address[5] = { 0xE2, 0xE4, 0x23, 0xE4, 0x02 };
      nrf_init(address);
     
      Serial.begin(57600);
      Serial.println("Init");
    }

    void loop()
    {
      Serial.println("Waiting for a packet...");
     
      do {} while (!(nrf24l01p_get_irq_flags() & (1 << NRF24L01P_IRQ_RX_DR)));
      nrf24l01p_clear_irq_flag(NRF24L01P_IRQ_RX_DR);
     
      static char payload[NRF_PAYLOAD_LENGTH];
      nrf24l01p_read_rx_payload((uint8_t*)payload);//, sizeof(payload));

      Serial.print(">>");
      Serial.println(payload);
    }

    void nrf_init(uint8_t *address)
    {
        delay(100);

        nrf24l01p_get_clear_irq_flags();
        nrf24l01p_close_pipe(NRF24L01P_ALL);
        nrf24l01p_open_pipe(NRF24L01P_PIPE0, false);
        nrf24l01p_set_crc_mode(NRF24L01P_CRC_16BIT);
        nrf24l01p_set_address_width(NRF24L01P_AW_5BYTES);
        nrf24l01p_set_address(NRF24L01P_PIPE0, address);
        nrf24l01p_set_operation_mode(NRF24L01P_PRX);
        nrf24l01p_set_rx_payload_width(NRF24L01P_PIPE0, NRF_PAYLOAD_LENGTH);
        nrf24l01p_set_rf_channel(NRF_CHANNEL);

        nrf24l01p_set_power_mode(NRF24L01P_PWR_UP);
        delay(NRF_POWER_UP_DELAY);
    }

    void nrf24l01p_spi_ss(nrf24l01p_spi_ss_level_t level)
    {
      digitalWrite(SPI_SS_PIN, (level == NRF24L01P_SPI_SS_LOW ? LOW : HIGH));
    }

    uint8_t nrf24l01p_spi_rw(uint8_t value)
    {
      return SPI.transfer(value);
    }
