async def test_add_basket_missing_field(service_client):
    response = await service_client.post(
        '/add_basket',
        json={'sellerId': 1, 'buyerId': 2, 'variantId': 3},
    )
    assert response.status == 400


async def test_add_basket_invalid_status(service_client):
    response = await service_client.post(
        '/add_basket',
        json={
            'sellerId': 1,
            'buyerId': 2,
            'variantId': 3,
            'price': 100,
            'status': 'not-a-real-status',
        },
    )
    assert response.status == 400


async def test_add_basket_success(service_client):
    response = await service_client.post(
        '/add_basket',
        json={'sellerId': 1, 'buyerId': 2, 'variantId': 3, 'price': 150},
    )
    assert response.status == 200
    assert response.json()['status'] == 'ok'
