
create database if not exsits back_stream;
use back_stream;
grant all on back_stream.* to root@'%' identified by 'root';

drop table if exsits RecordGold;
create table if not exsits RecordMoney(
	id bigInt auto_increment unique key,
    role_id bigInt default 0,
    role_name varChar(100) default NULL,
    serial_type int default 0,
    sub_serial_type int default 0,
    amount int default 0,
    tatol_amount int default 0,
    time int default 0,
	primary key (`id`),
	key `role_id`(`role_id`,`role_name`),
	key `serial_type`(`serial_type`)
) ENGINE=innoDB DEFAULT CHARSET=utf8;

drop table if exsits RecordCopper;
create table if not exsits RecordMoney(
	id bigInt auto_increment unique key,
    role_id bigInt default 0,
    role_name varChar(100) default NULL,
    serial_type int default 0,
    sub_serial_type int default 0,
    amount int default 0,
    tatol_amount int default 0,
    time int default 0,
	primary key (`id`),
	key `role_id`(`role_id`,`role_name`),
	key `serial_type`(`serial_type`)
) ENGINE=innoDB DEFAULT CHARSET=utf8;


drop table if exsits RecordCoupon;
create table if not exsits RecordMoney(
	id bigInt auto_increment unique key,
    role_id bigInt default 0,
    role_name varChar(100) default NULL,
    serial_type int default 0,
    sub_serial_type int default 0,
    amount int default 0,
    tatol_amount int default 0,
    time int default 0,
	primary key (`id`),
	key `role_id`(`role_id`,`role_name`),
	key `serial_type`(`serial_type`)
) ENGINE=innoDB DEFAULT CHARSET=utf8;

