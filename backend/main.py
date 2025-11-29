from flask import Flask, request, jsonify

from src.model import CapacityMeasurement
from src.influx import get_write_api, get_bucket

app = Flask(__name__)

write_api = get_write_api()
bucket = get_bucket()


@app.route("/", methods=["GET"])
def health():
    return jsonify({"status": "ok"}), 200


@app.route("/telemetry", methods=["POST"])
def telemetry():
    """
    Espera um JSON do ESP no formato, por exemplo:

    {
        "capacity_pct": 73.2,     // 0-100 (% cheio)": 72.5,
        "below_limit": false,
        "timestamp": 1710000000  // opcional (epoch s ou ms)
    }
    """
    try:
        data = request.get_json(force=True)
    except Exception:
        return jsonify({"error": "JSON inválido"}), 400

    if not isinstance(data, dict):
        return jsonify({"error": "Corpo da requisição deve ser um objeto JSON"}), 400

    # montar o modelo a partir do payload
    try:
        measurement = CapacityMeasurement.from_payload(data)
    except KeyError as e:
        return jsonify({"error": f"Campo obrigatório ausente: {e}"}), 400
    except ValueError as e:
        return jsonify({"error": f"Valor inválido: {e}"}), 400

    # converte pro InfluxDB
    point = measurement.to_point()

    # escreve no Influx
    try:
        write_api.write(bucket=bucket, record=point)
    except Exception as e:
        return jsonify({"error": f"Falha ao escrever no InfluxDB: {e}"}), 500

    return jsonify({"status": "saved"}), 201


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=8000, debug=True)
