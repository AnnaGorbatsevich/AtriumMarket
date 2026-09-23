async def test_order_then_cancel_restores_availability(service_client):
    add_response = await service_client.post(
        '/add_product',
        json={
            'sellerId': 501,
            'name': 'Availability Product',
            'variants': [{'price': 100, 'quantity': 5}],
        },
    )
    assert add_response.status == 200

    products = await service_client.get('/products', params={'sellerId': 501})
    variant_id = products.json()[0]['variants'][0]['id']

    order_response = await service_client.post(
        '/update_availability',
        json={'variantId': variant_id, 'quantity': -2},
    )
    assert order_response.status == 200
    assert order_response.json()['remaining'] == 3

    products = await service_client.get('/products', params={'sellerId': 501})
    assert products.json()[0]['variants'][0]['quantity'] == 3
