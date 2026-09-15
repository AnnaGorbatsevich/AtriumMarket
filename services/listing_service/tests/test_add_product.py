async def test_add_product_missing_name(service_client):
    response = await service_client.post(
        '/add_product',
        json={'sellerId': 1, 'variants': [{'price': 100, 'quantity': 1}]},
    )
    assert response.status == 400


async def test_add_product_missing_variants(service_client):
    response = await service_client.post(
        '/add_product',
        json={'sellerId': 1, 'name': 'No variants'},
    )
    assert response.status == 400


async def test_add_product_negative_variant_price(service_client):
    response = await service_client.post(
        '/add_product',
        json={
            'sellerId': 1,
            'name': 'Bad variant',
            'variants': [{'price': -1, 'quantity': 1}],
        },
    )
    assert response.status == 400


async def test_add_product_invalid_status(service_client):
    response = await service_client.post(
        '/add_product',
        json={
            'sellerId': 1,
            'name': 'Bad status',
            'status': 'not-a-real-status',
            'variants': [{'price': 100, 'quantity': 1}],
        },
    )
    assert response.status == 400


async def test_add_product_success(service_client):
    response = await service_client.post(
        '/add_product',
        json={
            'sellerId': 42,
            'name': 'Test Product',
            'variants': [{'price': 500, 'quantity': 3}],
        },
    )
    assert response.status == 200
    assert response.json()['status'] == 'ok'

    products = await service_client.get('/products', params={'sellerId': 42})
    assert products.status == 200
    body = products.json()
    assert len(body) == 1
    assert body[0]['name'] == 'Test Product'
    assert body[0]['variants'][0]['price'] == 500
    assert body[0]['variants'][0]['quantity'] == 3


async def test_products_requires_seller_id(service_client):
    response = await service_client.get('/products')
    assert response.status == 400


async def test_products_unknown_seller_is_empty(service_client):
    response = await service_client.get('/products', params={'sellerId': 999999})
    assert response.status == 200
    assert response.json() == []
