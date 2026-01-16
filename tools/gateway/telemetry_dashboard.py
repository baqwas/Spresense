#!/usr/bin/env python3
import serial
import json
import time
from rich.live import Live
from rich.table import Table
from rich.layout import Layout
from rich.panel import Panel
from rich.console import Console

# --- Configuration ---
SERIAL_PORT = '/dev/ttyUSB0'  # Update this to your Spresense port
BAUD_RATE = 115200

# Mapping based on Sony Spresense SDK IDs
TYPE_MAP = {
    0: "GPS/QZSS",
    1: "GLONASS",
    6: "BEIDOU",
    7: "GALILEO"
}

def create_dashboard(data):
    """Generates the Rich UI layout."""
    layout = Layout()
    
    # Header Panel
    fix_status = "FIX 3D" if data.get("Pos", {}).get("hdop", 99) < 5.0 else "SEARCHING"
    header = Panel(
        f"[bold cyan]SPRESENSE GNSS GATEWAY[/] | Status: [bold green]{fix_status}[/] | Sats: {data.get('Count', 0)}",
        style="white on blue"
    )
    
    # Satellite Breakdown Table
    sat_table = Table(title="Satellite Inventory", expand=True)
    sat_table.add_column("SVID", justify="right")
    sat_table.add_column("Type", style="magenta")
    sat_table.add_column("Signal (dBm)", justify="center")
    sat_table.add_column("Elevation°", justify="right")
    
    # Count types for summary
    counts = {"GPS/QZSS": 0, "GLONASS": 0, "BEIDOU": 0, "GALILEO": 0, "Unknown": 0}
    
    for sat in data.get("Sats", []):
        t_name = TYPE_MAP.get(sat['t'], "Unknown")
        counts[t_name] += 1
        # Color signal based on strength
        sig = sat['s']
        sig_col = "green" if sig > 35 else "yellow" if sig > 25 else "red"
        
        sat_table.add_row(
            str(sat['id']), 
            t_name, 
            f"[{sig_col}]{sig:.1f}[/]", 
            str(sat['el'])
        )

    # Position Info
    pos = data.get("Pos", {})
    pos_info = (
        f"UTC:  {data.get('UTC', 'N/A')}\n"
        f"LAT:  {pos.get('lat', 0):.6f}\n"
        f"LON:  {pos.get('lon', 0):.6f}\n"
        f"ALT:  {pos.get('alt', 0):.1f}m\n"
        f"HDOP: {pos.get('hdop', 0):.2f}"
    )
    
    # Summary of Constellations
    summary = " | ".join([f"{k}: {v}" for k, v in counts.items() if v > 0])
    
    layout.split_column(
        Layout(header, size=3),
        Layout(name="main")
    )
    layout["main"].split_row(
        Layout(Panel(pos_info, title="Position Details"), size=40),
        Layout(Panel(sat_table, title=f"Constellations ({summary})"))
    )
    
    return layout

def main():
    console = Console()
    try:
        ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=1)
        console.print(f"[green]Connected to Spresense on {SERIAL_PORT}[/]")
    except Exception as e:
        console.print(f"[red]Error: Could not open port {SERIAL_PORT}. {e}[/]")
        return

    with Live(console=console, screen=True, refresh_per_second=2) as live:
        while True:
            line = ser.readline().decode('utf-8', errors='ignore').strip()
            if line.startswith('{'):
                try:
                    data = json.loads(line)
                    live.update(create_dashboard(data))
                except json.JSONDecodeError:
                    continue

if __name__ == "__main__":
    main()
