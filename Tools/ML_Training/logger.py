import serial
import serial.tools.list_ports
import csv
import sys
from datetime import datetime
from pathlib import Path

EXPECTED_FIELDS = 6
CSV_HEADER = ["acc_x", "acc_y", "acc_z", "gyro_x", "gyro_y", "gyro_z"]

BASE_DIR = Path(__file__).resolve().parent
DATA_DIR = BASE_DIR / "data"


# ---------------- SERIAL PORT SELECTION ---------------- #

def select_serial_port():
    ports = list(serial.tools.list_ports.comports())

    if not ports:
        print("No COM ports found.")
        sys.exit(1)

    com_map = {}

    print("\nAvailable COM ports:")

    for port in ports:
        name = port.device  # e.g. COM5 or /dev/ttyUSB0

        if name.upper().startswith("COM"):
            try:
                number = int(name[3:])
                com_map[number] = name
                print(f"COM{number}  |  {port.description}")
            except ValueError:
                continue
        else:
            # Non-Windows fallback display
            print(f"{name}  |  {port.description}")

    if not com_map:
        # Non-Windows case → ask full name
        return input("\nEnter full port name: ").strip()

    while True:
        try:
            user_input = int(input("\nEnter COM port number: "))

            if user_input in com_map:
                return com_map[user_input]
            else:
                print("This COM number is not in the available list.")

        except ValueError:
            print("Please enter a valid number.")


def get_baud_rate():
    while True:
        baud = input("Enter baud rate (default 115200): ").strip()

        if baud == "":
            return 115200

        try:
            return int(baud)
        except ValueError:
            print("Invalid baud rate. Enter a number.")


# ---------------- FILE NAME ---------------- #

def get_filename():
    label = input("Enter label for this dataset: ").strip().replace(" ", "_")
    if not label:
        label = "unlabeled"
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")

    # Create base data directory if needed
    DATA_DIR.mkdir(exist_ok=True)

    # Create label-specific subfolder
    label_dir = DATA_DIR / label
    label_dir.mkdir(exist_ok=True)

    return label_dir / f"{timestamp}_{label}.csv"


# ---------------- DATA PARSING ---------------- #

def parse_line(line):
    parts = line.strip().split()

    if len(parts) != EXPECTED_FIELDS:
        return None

    try:
        return [float(x) for x in parts]
    except ValueError:
        return None


# ---------------- MAIN ---------------- #

def main():
    port = select_serial_port()
    baud = get_baud_rate()
    filepath = get_filename()

    try:
        ser = serial.Serial(port, baud, timeout=1)
        print(f"\nOpened {port} @ {baud}\n")
    except serial.SerialException as e:
        print(f"Error opening serial port: {e}")
        sys.exit(1)

    try:
        with open(filepath, mode="w", newline="") as f:
            writer = csv.writer(f)
            writer.writerow(CSV_HEADER)

            print("Logging started. Press Ctrl+C to stop.\n")

            while True:
                try:
                    line = ser.readline().decode("utf-8", errors="ignore")

                    if not line:
                        continue

                    parsed = parse_line(line)

                    if parsed:
                        writer.writerow(parsed)
                        print(parsed)
                    else:
                        print(f"Ignored: {line.strip()}")

                except KeyboardInterrupt:
                    print("\nLogging stopped by user.")
                    break

    finally:
        ser.close()
        print(f"\nFile saved at:\n{filepath}")


if __name__ == "__main__":
    main()