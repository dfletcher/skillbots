
var webserver = require('./webserver'),
    gameworker = require('./gameworker');

gameworker.run();
webserver.run(gameworker);
