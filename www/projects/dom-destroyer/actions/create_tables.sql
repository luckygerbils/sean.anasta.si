
DROP TABLE IF EXISTS scores;
CREATE TABLE scores (
    id      INT         PRIMARY KEY AUTO_INCREMENT
  , ip      VARCHAR(15) NOT NULL UNIQUE
  , score   INT         NOT NULL
);
