INSERT INTO delivery_service.courier(id, region, transport, working_hours)
VALUES
    ('1', '1', 'пеший', '8:00-20:00');

INSERT INTO delivery_service.order(id, region, weight, delivery_hours, begin_date, end_date, price, courier_id, completed_time, completed_day)
VALUES
    ('1', '1', '60', '20:00-21:00', '1024', '1', '0', '0000-00-00'),
    ('2', '1', '60', '10:00-11:00', '01.02.2003', '1024', '1', '0', '0000-00-00');
