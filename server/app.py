import os
from flask import Flask, request, jsonify, render_template, redirect, url_for
from flask_sqlalchemy import SQLAlchemy
from datetime import datetime

app = Flask(__name__)
app.config['SQLALCHEMY_DATABASE_URI'] = 'sqlite:///scale_data.db'
app.config['SQLALCHEMY_TRACK_MODIFICATIONS'] = False
db = SQLAlchemy(app)

# --- Database Models ---

class WeightRecord(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    timestamp = db.Column(db.String(50), nullable=False)
    weight_g = db.Column(db.Float, nullable=False)
    received_at = db.Column(db.DateTime, default=datetime.utcnow)

class DeviceConfig(db.Model):
    id = db.Column(db.Integer, primary_key=True)
    # The ESP8266 will read these if update_pending is True
    update_pending = db.Column(db.Boolean, default=False)
    telemetry_enabled = db.Column(db.Boolean, default=True)
    ntp_server = db.Column(db.String(100), default="pool.ntp.org")
    sleep_interval_sec = db.Column(db.Integer, default=3600)

with app.app_context():
    db.create_all()
    # Initialize config if empty
    if not DeviceConfig.query.first():
        default_config = DeviceConfig()
        db.session.add(default_config)
        db.session.commit()

# --- Web UI Routes ---

@app.route('/')
def index():
    records = WeightRecord.query.order_by(WeightRecord.received_at.desc()).limit(20).all()
    config = DeviceConfig.query.first()
    return render_template('index.html', records=records, config=config)

@app.route('/update_config', methods=['POST'])
def update_config():
    config = DeviceConfig.query.first()
    config.telemetry_enabled = 'telemetry_enabled' in request.form
    config.ntp_server = request.form.get('ntp_server', 'pool.ntp.org')
    
    # Whenever the UI updates the config, we set update_pending to True
    # so the ESP8266 knows it needs to apply these changes on its next wakeup.
    config.update_pending = True
    
    db.session.commit()
    return redirect(url_for('index'))

# --- API Routes for ESP8266 ---

@app.route('/api/weight', methods=['POST'])
def receive_weight():
    data = request.get_json()
    if not data:
        return jsonify({"error": "No JSON payload"}), 400

    # 1. Save the incoming weight reading
    timestamp = data.get('timestamp', 'Unknown')
    weight_g = data.get('weight_g', 0.0)

    record = WeightRecord(timestamp=timestamp, weight_g=weight_g)
    db.session.add(record)
    
    # 2. Check for pending configurations to send back to the ESP8266
    config = DeviceConfig.query.first()
    response_payload = {
        "status": "success",
        "update_config": config.update_pending
    }

    if config.update_pending:
        response_payload["config"] = {
            "telemetry": "on" if config.telemetry_enabled else "off",
            "ntp_server": config.ntp_server
        }
        # Reset the flag since we're dispatching it now
        config.update_pending = False

    db.session.commit()
    
    return jsonify(response_payload), 200

if __name__ == '__main__':
    # Listen on all interfaces so the ESP8266 can reach it
    app.run(host='0.0.0.0', port=5000, debug=True)
