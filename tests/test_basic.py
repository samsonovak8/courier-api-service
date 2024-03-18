import pytest

from testsuite.databases import pgsql


# Start the tests via `make test-debug` or `make test-release`


async def test_add_courier(service_client):
    data = {"region": "1", "transport": "велокурьер", "working_hours": "10:00-12:00"}
    response = await service_client.post(
        '/v1/couriers',
        json=data,
    )
    assert response.status == 200
    # assert response.text == 'Hello, userver!\n'

async def test_add_courier_invalid_input(service_client):
    data = {"region": "1jhdsf", "transport": "велокурьер", "working_hours": "10:00-12:00"}
    response = await service_client.post(
        '/v1/couriers',
        json=data,
    )
    assert response.status == 400
    data = {"region": "1", "transport": "велкурьер", "working_hours": "10:00-12:00"}
    response = await service_client.post(
        '/v1/couriers',
        json=data,
    )
    assert response.status == 400
    data = {"region": "1", "transport": "велокурьер", "working_hours": "10:70-60:00"}
    response = await service_client.post(
        '/v1/couriers',
        json=data,
    )
    assert response.status == 400

async def test_add_courier_without_required_params(service_client):
    data = {"transport": "велокурьер", "working_hours": "10:00-12:00"}
    response = await service_client.post(
        '/v1/couriers',
        json=data,
    )
    assert response.status == 400
    data = {"region": "1", "working_hours": "10:00-12:00"}
    response = await service_client.post(
        '/v1/couriers',
        json=data,
    )
    assert response.status == 400
    data = {"region": "1", "transport": "велокурьер"}
    response = await service_client.post(
        '/v1/couriers',
        json=data,
    )
    assert response.status == 400




# async def test_db_updates(service_client):
#     response = await service_client.post('/v1/hello', params={'name': 'World'})
#     assert response.status == 200
#     assert response.text == 'Hello, World!\n'

#     response = await service_client.post('/v1/hello', params={'name': 'World'})
#     assert response.status == 200
#     assert response.text == 'Hi again, World!\n'

#     response = await service_client.post('/v1/hello', params={'name': 'World'})
#     assert response.status == 200
#     assert response.text == 'Hi again, World!\n'


# @pytest.mark.pgsql('db_1', files=['initial_data.sql'])
# async def test_db_initial_data(service_client):
#     response = await service_client.post(
#         '/v1/hello',
#         params={'name': 'user-from-initial_data.sql'},
#     )
#     assert response.status == 200
#     assert response.text == 'Hi again, user-from-initial_data.sql!\n'
