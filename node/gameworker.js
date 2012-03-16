var DATABASE = 'mak';
var DATABASE_USER = 'root';
var DATABASE_PASS = '';
var _mysql = require('mysql'),
    util = require('util');

var mysql = _mysql.createClient({
  user: DATABASE_USER,
  password: DATABASE_PASS,
});

var gameworker = {
  process: null,
  started: null,
};

function gameloop() {
  console.log('gameworker: ' + util.inspect(gameworker));
  setTimeout(gameloop, 1000);
}

var run = function() {
  mysql.query('USE ' + DATABASE);
  setTimeout(gameloop, 0);
};

exports.run = run;
