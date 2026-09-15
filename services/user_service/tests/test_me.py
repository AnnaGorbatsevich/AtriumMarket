async def test_me_returns_profile(service_client):
    email = 'me_test@example.com'
    register_response = await service_client.post(
        '/register',
        json={
            'role': 'seller',
            'fullName': 'Me Test',
            'email': email,
            'phone': '+70000000000',
            'password': 'hunter2',
        },
    )
    assert register_response.status == 200
    token = register_response.json()['token']

    response = await service_client.get(
        '/me', headers={'Authorization': f'Bearer {token}'}
    )
    assert response.status == 200
    body = response.json()
    assert body['email'] == email
    assert body['fullName'] == 'Me Test'
    assert body['role'] == 'seller'
    assert isinstance(body['id'], int)


async def test_me_without_token(service_client):
    response = await service_client.get('/me')
    assert response.status == 401


async def test_me_with_invalid_token(service_client):
    response = await service_client.get(
        '/me', headers={'Authorization': 'Bearer not-a-real-token'}
    )
    assert response.status == 401
