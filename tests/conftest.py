def pytest_configure(config):
    config.addinivalue_line(
        "markers", "remote_data: mark tests that require remote data"
    ) 