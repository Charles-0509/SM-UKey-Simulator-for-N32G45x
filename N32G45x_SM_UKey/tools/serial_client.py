import argparse
import sys
import time

try:
    import serial
except ImportError:
    serial = None


def main():
    parser = argparse.ArgumentParser(description="SM UKey USB CDC serial client")
    parser.add_argument("port", help="COM port, for example COM5")
    parser.add_argument("-b", "--baudrate", type=int, default=115200)
    parser.add_argument("-c", "--command", action="append", help="command to send; may be repeated")
    args = parser.parse_args()

    if serial is None:
        print("pyserial is required: pip install pyserial", file=sys.stderr)
        return 2

    with serial.Serial(args.port, args.baudrate, timeout=1) as ser:
        commands = args.command or []
        if not commands:
            print("Interactive mode. Ctrl+C to exit.")
            while True:
                cmd = input("> ")
                ser.write((cmd + "\r\n").encode("ascii"))
                print(ser.readline().decode("ascii", errors="replace").strip())
        else:
            for cmd in commands:
                ser.write((cmd + "\r\n").encode("ascii"))
                time.sleep(0.05)
                print(ser.readline().decode("ascii", errors="replace").strip())


if __name__ == "__main__":
    raise SystemExit(main())

