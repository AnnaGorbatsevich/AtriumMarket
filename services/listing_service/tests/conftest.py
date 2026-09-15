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
        'listing_service',
        [pathlib.Path(__file__).parent.parent / 'db'],
    )
    return pgsql_local_create(list(databases.values()))
