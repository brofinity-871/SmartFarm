const express = require('express');
const mysql = require('mysql2');
const app = express();

app.use(express.json());

// DATABASE CONNECTION

const db = mysql.createConnection({
	host: 'localhost',
	user: 'root',
	password: 'syStemHlr_871',
	database: 'smartfarm'
});

db.connect(err => {
	if (err) {
		console.error('Database connection failed:', err);
		return;
	}
	console.log('Connected to MySQL');
});

// API ENDPOINT
app.post('/api/lux_snapshot', (req, res) => {
	console.log("Incoming request:", req.body);
	const { esp_id, last_snap, mean_lux, min_lux, max_lux, sample_count } = req.body;

	// Basic validation
	if (!esp_id || !last_snap || mean_lux == null) {
		return res.status(400).json({error: 'Invalid payload'});
	}

	const sql = `INSERT INTO lux_snapshots(esp_id, last_snap, mean_lux, min_lux, max_lux, sample_count)
		VALUES (?, ?, ?, ?, ?, ?)
	`;

	db.query(sql,[esp_id, last_snap, mean_lux, min_lux, max_lux, sample_count], (err, result) => {
		if(err) {
			console.error('Insert error:', err);
			return res.status(500).json({ error: err.message });
		}
		res.status(200).json({ success: true });
	}
	);
});

// DEBUG
app.get('/debug/lux', (req, res) => {
	db.query("SELECT * FROM lux_snapshots ORDER BY last_snap DESC LIMIT 10", (err, results) => {
		if (err) return res.status(500).json(err);
		res.json(results);
	});
});

// START SERVER
app.listen(3000, () => {
	console.log('Server running on port 3000');
});















