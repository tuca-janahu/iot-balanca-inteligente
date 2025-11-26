import os
from dotenv import load_dotenv
from influxdb_client import InfluxDBClient
from influxdb_client.client.write_api import SYNCHRONOUS

# Carrega variáveis do arquivo .env na raiz do projeto
load_dotenv()

# Variáveis de ambiente necessárias
INFLUX_URL = os.getenv("INFLUX_URL")
INFLUX_TOKEN = os.getenv("INFLUX_TOKEN")
INFLUX_ORG = os.getenv("INFLUX_ORG")
INFLUX_BUCKET = os.getenv("INFLUX_BUCKET")

if not INFLUX_TOKEN:
    raise RuntimeError("A variável de ambiente INFLUX_TOKEN não está definida.")

if not INFLUX_ORG:
    raise RuntimeError("A variável de ambiente INFLUX_ORG não está definida.")

if not INFLUX_BUCKET:
    raise RuntimeError("A variável de ambiente INFLUX_BUCKET não está definida.")

# Cliente global simples (para projeto pequeno está ok)
_client = InfluxDBClient(url=INFLUX_URL, token=INFLUX_TOKEN, org=INFLUX_ORG)
_write_api = _client.write_api(write_options=SYNCHRONOUS)
_query_api = _client.query_api()


def get_write_api():
    """Retorna a API de escrita do InfluxDB."""
    return _write_api


def get_query_api():
    """Retorna a API de consulta do InfluxDB (se você quiser usar depois)."""
    return _query_api


def get_bucket():
    """Bucket padrão onde vamos escrever."""
    return INFLUX_BUCKET


def get_org():
    """Org padrão configurada no Influx."""
    return INFLUX_ORG