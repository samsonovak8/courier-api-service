INSERT INTO delivery_service.courier(id, region, transport, working_hours)
VALUES
    ('1', '1', 'пеший', '8:00-20:00');

INSERT INTO delivery_service.order(id, region, weight, delivery_hours, price, courier_id, completed_time)
VALUES
    ('1', '1', '60', '10:00-11:00', '1024', '1', '0');