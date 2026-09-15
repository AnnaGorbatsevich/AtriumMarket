async def test_get_orders_requires_id(service_client):
    response = await service_client.get('/get_orders')
    assert response.status == 400


async def test_get_orders_buyer_sees_cart_items(service_client):
    add_response = await service_client.post(
        '/add_basket',
        json={'sellerId': 11, 'buyerId': 21, 'variantId': 31, 'quantity': 1, 'price': 250},
    )
    assert add_response.status == 200

    response = await service_client.get('/get_orders', params={'buyerId': 21})
    assert response.status == 200
    items = response.json()
    assert len(items) == 1
    assert items[0]['sellerId'] == 11
    assert items[0]['variantId'] == 31


async def test_get_orders_seller_excludes_cart_items(service_client):
    add_response = await service_client.post(
        '/add_basket',
        json={'sellerId': 12, 'buyerId': 22, 'variantId': 32, 'quantity': 1, 'price': 250},
    )
    assert add_response.status == 200

    response = await service_client.get('/get_orders', params={'sellerId': 12})
    assert response.status == 200
    assert response.json() == []
