import pytest

pytest_plugins = ['pytest_userver.plugins.core']


@pytest.fixture(scope='session')
def service_env() -> dict:
    return {'JWT_SECRET': 'test-secret'}
