from dataclasses import dataclass
from datetime import datetime, timezone
from typing import Any, Optional

from influxdb_client import Point


MEASUREMENT_NAME = "tank_capacity_monitor"  # tabela no Influx


@dataclass
class CapacityMeasurement:
    """
    Representa uma leitura da capacidade do recipiente em porcentagem.
    """
    capacity_pct: float      # 0–100 (% cheio)
    below_limit: bool        # se está abaixo do limite definido no ESP
    timestamp: datetime      # momento da leitura

    @classmethod
    def from_payload(cls, data: dict[str, Any]) -> "CapacityMeasurement":
        """
        Cria o modelo a partir de um JSON enviado pelo ESP.

        Esperado algo como:
        {
            "capacity_pct": 73.2,   # 0–100
            "below_limit": false,   # calculado no ESP
            "timestamp": 1710000000 # opcional: epoch em segundos ou ms
        }
        """
        # Porcentagem (obrigatória)
        capacity_pct = float(data["capacity_pct"])

        # Flag abaixo do limite (obrigatória)
        raw_below = data["below_limit"]
        if not isinstance(raw_below, bool):
            raise ValueError("below_limit deve ser booleano (true/false no JSON).")
        below_limit = raw_below
        
        # SEMPRE usar o horário atual
        ts = datetime.now(timezone.utc)

        return cls(
            capacity_pct=capacity_pct,
            below_limit=below_limit,
            timestamp=ts,
        )

    def to_point(self) -> Point:
        """
        Converte a leitura em um Point do InfluxDB.
        """
        return (
            Point(MEASUREMENT_NAME)
            .field("capacity_pct", self.capacity_pct)
            .field("below_limit", self.below_limit)
            .time(self.timestamp)
        )
