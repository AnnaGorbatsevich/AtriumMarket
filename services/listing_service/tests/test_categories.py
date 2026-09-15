async def test_categories(service_client, pgsql):
    cursor = pgsql['init'].cursor()
    cursor.execute("INSERT INTO categories (name) VALUES ('Root') RETURNING id")
    root_id = cursor.fetchone()[0]
    cursor.execute(
        'INSERT INTO categories (name, parent_id) VALUES (%s, %s)',
        ('Child', root_id),
    )

    response = await service_client.get('/categories')
    assert response.status == 200

    categories = {category['name']: category for category in response.json()}
    assert 'Root' in categories
    assert 'Child' in categories
    assert categories['Root']['parentId'] is None
    assert categories['Child']['parentId'] == root_id
