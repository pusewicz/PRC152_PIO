import pytest
from dc_client import DevConsole, SerialTransport, HttpTransport


def pytest_addoption(parser):
    parser.addoption("--serial", help="serial port, e.g. /dev/cu.usbserial-0001")
    parser.addoption("--wifi", help="radio IP, e.g. 192.168.152.1")


@pytest.fixture(scope="session")
def dc(request):
    port = request.config.getoption("--serial")
    host = request.config.getoption("--wifi")
    if port:
        return DevConsole(SerialTransport(port))
    if host:
        return DevConsole(HttpTransport(host))
    pytest.skip("no --serial or --wifi transport given (hardware required)")
