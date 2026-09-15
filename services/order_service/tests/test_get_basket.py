async def test_get_basket_requires_seller_id(service_client):
    response = await service_client.get('/get_basket')
    assert response.status == 400


async def test_get_basket_returns_added_item(service_client):
    add_response = await service_client.post(
        '/add_basket',
        json={'sellerId': 10, 'buyerId': 20, 'variantId': 30, 'quantity': 1, 'price': 200},
    )
    assert add_response.status == 200

    response = await service_client.get('/get_basket', params={'sellerId': 20})
    assert response.status == 200
    items = response.json()
    assert len(items) == 1
    assert items[0]['buyerId'] == 20
    assert items[0]['sellerId'] == 10
    assert items[0]['variantId'] == 30
    assert items[0]['quantity'] == 1
    assert items[0]['price'] == 200


async def test_get_basket_unknown_buyer_is_empty(service_client):
    response = await service_client.get('/get_basket', params={'sellerId': 999999})
    assert response.status == 200
    assert response.json() == []
