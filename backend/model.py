from dataclasses import dataclass
from datetime import datetime, timezone
from typing import Any, Optional
from influxdb_client import Point


MEASUREMENT_NAME = "tabela_leitura_peso"  


@dataclass
class WeightMeasurement:
    """
    Representa uma leitura da balança.
    """
    weight_kg: float
    below_limit: bool
    timestamp: datetime

    @classmethod
    def from_payload(cls, data: dict[str, Any]) -> "WeightMeasurement":
        """
        Cria o modelo a partir de um JSON enviado pelo ESP.

        Esperado algo como:
        {
            "weight": 72.5,          # em kg
            "below_limit": false,    # calculado no ESP
            "timestamp": 1710000000  # opcional: epoch em segundos ou ms
        }
        """
        # Peso (obrigatório)
        weight = float(data["weight"])
        below_limit = bool(data["below_limit"])
        ts_raw: Optional[Any] = data.get("timestamp")

        if ts_raw is None:
            # Se o ESP não mandar timestamp, usamos o horário do servidor
            ts = datetime.now(timezone.utc)
        else:
            # Aceita epoch em segundos ou milissegundos
            ts_float = float(ts_raw)
            if ts_float > 10_000_000_000:  # heurística simples: ms
                ts_float /= 1000.0
            ts = datetime.fromtimestamp(ts_float, tz=timezone.utc)

        return cls(
            weight_kg=weight,
            below_limit=below_limit,
            timestamp=ts,
        )

    def to_point(self) -> Point:
        """
        Converte a leitura em um Point do InfluxDB.
        """
        return (
            Point(MEASUREMENT_NAME)
            .field("weight_kg", self.weight_kg)
            .field("below_limit", self.below_limit)  # Influx aceita bool como field
            .time(self.timestamp)
        )