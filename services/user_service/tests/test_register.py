async def test_register_returns_token(service_client):
    response = await service_client.post(
        '/register',
        json={
            'role': 'buyer',
            'fullName': 'Test User',
            'email': 'register_test@example.com',
            'phone': '+70000000000',
            'password': 'hunter2',
        },
    )
    assert response.status == 200
    body = response.json()
    assert body['status'] == 'ok'
    assert body['token']


async def test_register_missing_field(service_client):
    response = await service_client.post(
        '/register',
        json={
            'role': 'buyer',
            'fullName': 'Test User',
            'phone': '+70000000000',
            'password': 'hunter2',
        },
    )
    assert response.status == 400


async def test_register_duplicate_email(service_client):
    payload = {
        'role': 'buyer',
        'fullName': 'Test User',
        'email': 'duplicate_test@example.com',
        'phone': '+70000000000',
        'password': 'hunter2',
    }
    first = await service_client.post('/register', json=payload)
    assert first.status == 200

    second = await service_client.post('/register', json=payload)
    assert second.status == 400
