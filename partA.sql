


DROP TABLE IF EXISTS user_calendar_events, contact_emails, user_contacts, user_emails, user_folders, folder_types, users;

CREATE TABLE IF NOT EXISTS `users`
(
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `username` VARCHAR(128) NOT NULL,
  `domain` VARCHAR(128) NOT NULL,
  `password` TEXT NOT NULL,
  `alias` VARCHAR(128) DEFAULT NULL,  
  `fullname` VARCHAR(128) DEFAULT NULL,
  `company` VARCHAR(128) DEFAULT NULL,
  `home_addr` VARCHAR(256) DEFAULT NULL,
  `business_addr` VARCHAR(256) DEFAULT NULL,
    
  PRIMARY KEY(`id`),
  
  KEY(`fullname`),
  
  UNIQUE KEY (`username`,`domain`),
  UNIQUE KEY (`alias`,`domain`),
  
  KEY(`username`),
  KEY(`domain`),
  KEY(`alias`),
  
  KEY (`company`),
  KEY (`home_addr`),
  KEY (`business_addr`)
  
) ENGINE=INNODB DEFAULT CHARSET=utf8;


CREATE TABLE IF NOT EXISTS `folder_types`
(
  `id` TINYINT UNSIGNED NOT NULL AUTO_INCREMENT,
  `type` VARCHAR(32) NOT NULL,
  
  PRIMARY KEY(`id`)
) ENGINE=INNODB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `user_folders`
(
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  
  `name` VARCHAR(128) NOT NULL,
  
  `fkey_parent_folder` INT UNSIGNED,  
  `fkey_user` INT UNSIGNED NOT NULL,
  `fkey_type` TINYINT UNSIGNED NOT NULL,
  
  PRIMARY KEY(`id`),
  
  UNIQUE KEY(`fkey_user`, `fkey_parent_folder`, `fkey_type`, `name`),
  
  FOREIGN KEY (`fkey_user`) REFERENCES `users`(`id`) ON DELETE CASCADE,
  FOREIGN KEY (`fkey_parent_folder`) REFERENCES `user_folders`(`id`) ON DELETE CASCADE,
  FOREIGN KEY (`fkey_type`) REFERENCES `folder_types`(`id`)
  
) ENGINE=INNODB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `user_emails`
(
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  
  `header` TEXT NOT NULL,
  `body` TEXT NOT NULL,

  `fkey_user` INT UNSIGNED NOT NULL,
  `fkey_folder` INT UNSIGNED NOT NULL,
  
  PRIMARY KEY (`id`),
  
  FULLTEXT (`header`),
  FULLTEXT (`body`),
  
  FOREIGN KEY (`fkey_user`) REFERENCES `users`(`id`) ON DELETE CASCADE,
  FOREIGN KEY (`fkey_folder`) REFERENCES `user_folders`(`id`) ON DELETE CASCADE
  
) ENGINE=INNODB DEFAULT CHARSET=utf8;


CREATE TABLE IF NOT EXISTS `user_contacts`
(
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
  `fullname` VARCHAR(128) NOT NULL,
  `company` VARCHAR(128) DEFAULT NULL,
  `home_addr` VARCHAR(256) DEFAULT NULL,
  `business_addr` VARCHAR(256) DEFAULT NULL,
  
  `fkey_user` INT UNSIGNED NOT NULL,
  `fkey_folder` INT UNSIGNED NOT NULL,
  
  PRIMARY KEY(`id`),
  
  KEY (`fullname`),  
  KEY (`company`),
  KEY (`home_addr`),
  KEY (`business_addr`),
  
  FOREIGN KEY (`fkey_user`) REFERENCES `users`(`id`) ON DELETE CASCADE,
  FOREIGN KEY (`fkey_folder`) REFERENCES `user_folders`(`id`) ON DELETE CASCADE
  
) ENGINE=INNODB DEFAULT CHARSET=utf8;

CREATE TABLE IF NOT EXISTS `contact_emails`
(
  `id` INT UNSIGNED NOT NULL AUTO_INCREMENT,
  `email` VARCHAR(256) NOT NULL,
  
  `fkey_contact` BIGINT UNSIGNED NOT NULL,
  
  PRIMARY KEY(`fkey_contact`,`id`),
  
  KEY(`email`),
  KEY(`id`),
  
  UNIQUE KEY(`fkey_contact`,`email`),
  
  FOREIGN KEY (`fkey_contact`) REFERENCES `user_contacts`(`id`) ON DELETE CASCADE
  
) ENGINE=INNODB DEFAULT CHARSET=utf8;


CREATE TABLE IF NOT EXISTS `user_calendar_events`
(
  `id` BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,  
  
  `title` VARCHAR(256) NOT NULL,
  `start` DATETIME NOT NULL,
  `end` DATETIME NOT NULL,
  `description` TEXT DEFAULT NULL,

  `fkey_user` INT UNSIGNED NOT NULL,
  `fkey_folder` INT UNSIGNED NOT NULL,
  
  PRIMARY KEY (`id`),
  
  KEY(`title`),
  KEY(`start`),
  KEY(`end`),
  
  FULLTEXT (`description`),
  
  FOREIGN KEY (`fkey_user`) REFERENCES `users`(`id`) ON DELETE CASCADE,
  FOREIGN KEY (`fkey_folder`) REFERENCES `user_folders`(`id`) ON DELETE CASCADE
  
) ENGINE=INNODB DEFAULT CHARSET=utf8;


INSERT INTO folder_types (`id`,`type`) VALUES
(1,'email'),
(2,'contact'),
(3,'calendar');

-- Trigger for creating root directories for new user
DELIMITER //
DROP TRIGGER IF EXISTS `create_user_root_dirs`//
CREATE TRIGGER `create_user_root_dirs` AFTER INSERT ON `users`
FOR EACH ROW
BEGIN
  INSERT INTO `user_folders` (`name`, `fkey_user`, `fkey_type`, `fkey_parent_folder`) VALUES
  ('E-mails',NEW.`id`, '1', NULL),
  ('Contacts',NEW.`id`, '2', NULL),
  ('Calendar',NEW.`id`, '3', NULL);
END//

DELIMITER ;


