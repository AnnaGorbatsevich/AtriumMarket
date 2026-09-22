async def _register(service_client, email, role='buyer', **extra):
    payload = {
        'role': role,
        'fullName': 'Old Name',
        'email': email,
        'phone': '+70000000000',
        'password': 'hunter2',
    }
    payload.update(extra)
    response = await service_client.post('/register', json=payload)
    assert response.status == 200
    return {'Authorization': f'Bearer {response.json()["token"]}'}


async def _update(service_client, headers, **fields):
    return await service_client.post('/update_profile', json=fields, headers=headers)


async def test_me_returns_profile_fields(service_client):
    headers = await _register(
        service_client,
        'profile_fields@example.com',
        role='seller',
        companyName='Shop',
        taxId='123',
        description='About',
    )

    response = await service_client.get('/me', headers=headers)
    assert response.status == 200
    body = response.json()
    assert body['phone'] == '+70000000000'
    assert body['companyName'] == 'Shop'
    assert body['taxId'] == '123'
    assert body['description'] == 'About'
    assert body['address'] is None


async def test_update_profile_buyer(service_client):
    headers = await _register(service_client, 'update_buyer@example.com', address='Old street')

    response = await _update(
        service_client, headers, fullName='New Name', phone='+79990001122', address='New street 1'
    )
    assert response.status == 200
    body = response.json()
    assert body['fullName'] == 'New Name'
    assert body['phone'] == '+79990001122'
    assert body['address'] == 'New street 1'
    assert body['email'] == 'update_buyer@example.com'
    assert body['role'] == 'buyer'

    me = (await service_client.get('/me', headers=headers)).json()
    assert me['fullName'] == 'New Name'
    assert me['address'] == 'New street 1'


async def test_update_profile_seller(service_client):
    headers = await _register(
        service_client, 'update_seller@example.com', role='seller', companyName='Shop', taxId='123'
    )

    response = await _update(
        service_client,
        headers,
        fullName='Contact',
        phone='+70001112233',
        companyName='New Shop',
        taxId='456',
        description='We sell things',
    )
    assert response.status == 200
    body = response.json()
    assert body['companyName'] == 'New Shop'
    assert body['taxId'] == '456'
    assert body['description'] == 'We sell things'


async def test_update_profile_absent_field_keeps_value_and_empty_clears(service_client):
    headers = await _register(
        service_client, 'update_keep@example.com', role='seller', companyName='Shop', taxId='123'
    )

    response = await _update(service_client, headers, fullName='Same', phone='+70000000000', taxId='')
    assert response.status == 200
    body = response.json()
    assert body['companyName'] == 'Shop'
    assert body['taxId'] is None


async def test_update_profile_requires_full_name_and_phone(service_client):
    headers = await _register(service_client, 'update_required@example.com')

    assert (await _update(service_client, headers, phone='+7')).status == 400
    assert (await _update(service_client, headers, fullName='Name')).status == 400
    assert (await _update(service_client, headers, fullName='', phone='+7')).status == 400


async def test_update_profile_without_token(service_client):
    response = await service_client.post('/update_profile', json={'fullName': 'A', 'phone': '1'})
    assert response.status == 401


async def test_update_profile_does_not_touch_other_users(service_client):
    headers = await _register(service_client, 'update_me@example.com')
    other = await _register(service_client, 'update_other@example.com')

    assert (await _update(service_client, headers, fullName='Changed', phone='+7')).status == 200

    other_me = (await service_client.get('/me', headers=other)).json()
    assert other_me['fullName'] == 'Old Name'
