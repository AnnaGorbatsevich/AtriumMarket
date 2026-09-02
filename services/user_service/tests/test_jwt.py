import base64
import hashlib
import hmac
import json
import re

JWT_SECRET = b'test-secret'


async def test_register(service_client):
    response = await service_client.post(
        '/register',
        json={
            'role': 'buyer',
            'fullName': 'Test User',
            'email': 'test@example.com',
            'phone': '+70000000000',
            'password': 'hunter2',
        },
    )
    assert response.status == 200

   