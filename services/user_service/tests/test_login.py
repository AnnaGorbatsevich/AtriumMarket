async def test_login_success(service_client):
    email = 'login_test@example.com'
    password = 'hunter2'
    register_response = await service_client.post(
        '/register',
        json={
            'role': 'buyer',
            'fullName': 'Login Test',
            'email': email,
            'phone': '+70000000000',
            'password': password,
        },
    )
    assert register_response.status == 200

    login_response = await service_client.post(
        '/login', json={'email': email, 'password': password}
    )
    assert login_response.status == 200
    body = login_response.json()
    assert body['status'] == 'ok'
    assert body['token']


async def test_login_wrong_password(service_client):
    email = 'login_wrong_pw@example.com'
    register_response = await service_client.post(
        '/register',
        json={
            'role': 'buyer',
            'fullName': 'Login Test',
            'email': email,
            'phone': '+70000000000',
            'password': 'correct-password',
        },
    )
    assert register_response.status == 200

    response = await service_client.post(
        '/login', json={'email': email, 'password': 'wrong-password'}
    )
    assert response.status == 400


async def test_login_unknown_email(service_client):
    response = await service_client.post(
        '/login',
        json={'email': 'no_such_user@example.com', 'password': 'whatever'},
    )
    assert response.status == 400
