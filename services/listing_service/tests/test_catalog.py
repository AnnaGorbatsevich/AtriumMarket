async def test_catalog_shows_active_products(service_client):
    add_response = await service_client.post(
        '/add_product',
        json={
            'sellerId': 7,
            'name': 'Catalog Product',
            'variants': [{'price': 300, 'quantity': 2}],
        },
    )
    assert add_response.status == 200

    catalog = await service_client.get('/catalog')
    assert catalog.status == 200
    names = {product['name'] for product in catalog.json()}
    assert 'Catalog Product' in names


async def test_catalog_hides_hidden_products(service_client):
    add_response = await service_client.post(
        '/add_product',
        json={
            'sellerId': 7,
            'name': 'Hidden Product',
            'status': 'hidden',
            'variants': [{'price': 300, 'quantity': 2}],
        },
    )
    assert add_response.status == 200

    catalog = await service_client.get('/catalog')
    assert catalog.status == 200
    names = {product['name'] for product in catalog.json()}
    assert 'Hidden Product' not in names

    products = await service_client.get('/products', params={'sellerId': 7})
    names = {product['name'] for product in products.json()}
    assert 'Hidden Product' in names
