-- CREATE TABLE `sensors_values` ( 
-- 	`datetime` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP , 
-- 	`temp` FLOAT NOT NULL , 
-- 	`humd` FLOAT NOT NULL , 
-- 	`fanspeed` SMALLINT NOT NULL,
-- 	`avglight` SMALLINT NOT NULL
-- );


CREATE TABLE `sensors_values` ( 
	`datetime` TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP , 
	`temp` FLOAT, 
	`humd` FLOAT, 
	`fanspeed` SMALLINT,
	`avglight` SMALLINT
);


INSERT INTO `sensors_values` (`datetime`, `temp`, `humd`, `fanspeed`, `avglight`) VALUES (current_timestamp(), '25.6', '84.3', '125', '240');


INSERT INTO `sensors_values` (`datetime`, `temp`, `humd`, `fanspeed`, `avglight`) VALUES ('2013-08-05 18:19:03', '25.6', '84.3', '125', '240');