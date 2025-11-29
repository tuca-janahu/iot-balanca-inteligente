# iot-balanca-inteligente

# 📦 SmartEstoque – Sistema IoT de Monitoramento de Estoques

O **SmartEstoque** é um sistema IoT desenvolvido para **monitorar automaticamente o nível de estoque em recipientes**, enviando os dados para um backend e exibindo dashboards em tempo real. Ele foi projetado para reduzir perdas, evitar falhas de reposição e otimizar a gestão de suprimentos em ambientes industriais e laboratoriais.

Este projeto foi desenvolvido como parte da matéria de **Práticas de IoT**.
Autores: **Ana Carolina Souza, Artur Janahú, Felipe Lucas, Maria Luiza Queiroz** 

---

# Visão Geral

O SmartEstoque tem como objetivos:

* Monitorar automaticamente o nível de estoque em tempo real.
* Reduzir desperdícios e erros de contagem.
* Gerar alertas quando o recipiente atingir níveis críticos.
* Disponibilizar dashboards acessíveis de qualquer lugar.
* Aumentar eficiência operacional.

Aplicações ideais:

* Indústria alimentícia
* Indústria química
* Indústria farmacêutica
* Armazéns e centros logísticos

---

# Arquitetura do Sistema

O projeto segue a arquitetura em **4 camadas IoT**:

---

## 1) Camada de Percepção – ESP 1 (Sensor)

**Componentes:**

* Sensor ultrassônico 
* LEDs indicadores
* Buzzer
* ESP32 (nó sensor)

**Funções:**

* Calcular distância → converter para nível → converter para % de capacidade.
* Determinar se o nível está abaixo do limite crítico.
* Publicar dados via MQTT para o ESP Gateway.

---

## 2) Camada de Rede – ESP 2 (Gateway)

**Função principal:**

* Receber dados do ESP 1 via MQTT.
* Interpretar JSON (`nivel` e `estado`).
* Converter para o formato do backend (`capacity_pct`, `below_limit`).
* Enviar via HTTP POST para o servidor Flask.

---

## 3) Camada de Serviço – Backend em Python


Tecnologias:

* **Flask** → API REST
* **InfluxDB** → Banco de dados time-series
* **Bucket:** `tank_capacity_monitor`
* Conexão via `influxdb-client-python`

Rotas principais:

```
POST /telemetry
{
  "capacity_pct": float,
  "below_limit": bool,
  "timestamp": int (optional)
}
```

Dados são armazenados com retenção adequada e timestamp validado.

---

## 4) Camada de Aplicação – Dashboard no Grafana

A visualização é feita com:

* **Grafana Cloud**
* Conexão com o mesmo Bucket InfluxDB
* Gráficos em tempo real:

  * Gauge do nível
  * Gráfico de linha da variação de capacidade
  * Indicador de crítico/normal


---

# Tecnologias Utilizadas

### Hardware

* ESP32 DevKit V1 (2 unidades)
* Sensor ultrassônico
* LEDs 
* Botão (interrupção externa)
* Buzzer
* Protoboard e jumpers

### Software

* Python 3.10+
* Flask
* InfluxDB Cloud
* Grafana Cloud
* Mosquitto MQTT Broker
* PubSubClient (Arduino)
* WiFi.h (Arduino)

---

# Como Rodar

## Backend
Para rodar o back, criamos um ambiente virtual que contém as importações necessárias para rodar o projeto, e definimos as variáveis de ambiente para conectar ao InfluxDB:

 * 1) Criar e ativar o virtual environment
```py
python -m venv .venv
.venv\Scripts\activate

```

 * 2) Instalar dependências do projeto
```bash
pip install dotenv influxdb_client flask

```
 * 3) Definir variáveis do influx no arquivo .env 
 (depois de defini-las no https://cloud2.influxdata.com)
```bash
INFLUX_URL=https://influxurl.com
INFLUX_TOKEN=seu-token
INFLUX_ORG=sua-org
INFLUX_BUCKET=seu-bucket
```
 * 4) Rodar o código
```bash
cd backend
python main.py
```
## MQTT (Mosquitto)
### Windows
* Instalar o mosquitto através do link: 
https://mosquitto.org/download/

### Linux
```bash
sudo apt update
sudo apt install mosquitto mosquitto-clients
```

## ESP1 (sensor)

* Coloque o IP do PC onde o Mosquitto está rodando em:

```cpp
const char* mqtt_server = "SEU_IPV4";
```

## ESP2 (gateway)

* Mesmo IP para MQTT
* Mesmo IP para backend:

```cpp
const char* backendUrl = "http://SEU_IPV4:8000/telemetry";
```

---

#  Conclusão

O SmartEstoque demonstra como integrar:

* Sensores
* MQTT
* HTTP
* Backend Python
* InfluxDB
* Grafana

num fluxo contínuo de monitoramento inteligente — aplicável para indústrias reais.

