import json
import pathlib

import pytest
from testsuite.databases.pgsql import discover

pytest_plugins = [
    'pytest_userver.plugins.core',
    'pytest_userver.plugins.postgresql',
]


@pytest.fixture(scope='session')
def pgsql_local(pgsql_local_create):
    databases = discover.find_schemas(
        'order_service',
        [pathlib.Path(__file__).parent.parent / 'db'],
    )
    return pgsql_local_create(list(databases.values()))


@pytest.fixture(scope='session')
def service_env():
    secdist = {
        'kafka_settings': {
            'kafka-producer': {
                'brokers': 'localhost:9092',
                'username': '',
                'password': '',
            },
        },
    }
    return {'SECDIST_CONFIG': json.dumps(secdist)}
