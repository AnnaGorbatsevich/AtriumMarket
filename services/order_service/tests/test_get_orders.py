async def _add(service_client, seller_id, buyer_id, variant_id):
    response = await service_client.post(
        '/add_basket',
        json={
            'sellerId': seller_id,
            'buyerId': buyer_id,
            'variantId': variant_id,
            'quantity': 1,
            'price': 250,
        },
    )
    assert response.status == 200

async def test_get_orders_requires_id(service_client):
    response = await service_client.get('/get_orders')
    assert response.status == 400


async def test_get_orders_buyer_excludes_cart_items(service_client):
    await _add(service_client, seller_id=11, buyer_id=21, variant_id=31)

    response = await service_client.get('/get_orders', params={'buyerId': 21})
    assert response.status == 200
    assert response.json() == []


async def test_get_orders_seller_excludes_cart_items(service_client):
    await _add(service_client, seller_id=12, buyer_id=22, variant_id=32)

    response = await service_client.get('/get_orders', params={'sellerId': 12})
    assert response.status == 200
    assert response.json() == []
