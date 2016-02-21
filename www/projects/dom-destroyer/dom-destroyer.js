// ==UserScript==
// @name        Dom Destroyer
// @namespace   sean.anasta.si
// @description Fight to defend your computer from the evil internet!
// @include     http://localhost:90/*
// @require     http://sean.anasta.si/js/jquery-1.4.2-GM.min.js
// @resource    stylesheet http://sean.anasta.si/projects/dom-destroyer/dom-destroyer.css
// ==/UserScript==
if (window.domDestroyer === undefined); // Only load once.
(function () {
    window.domDestroyer = true;

    // Create loading div immediately.
    var loading = document.createElement("div");
    loading.style.position        = 'fixed';
    loading.style.top             = '5px';
    loading.style.right           = '5px';
    loading.style.border          = '1px solid black';
    loading.style.backgroundColor = 'white';
    loading.style.padding         = '0.5em';
    loading.style.maxWidth        = '20em';
    loading.style.fontFamily      = "'DejaVu Sans', 'Bitstream Vera Sans', Verdana, sans-serif";
    loading.style.zIndex          = 101;
    loading.id                    = 'dom-destroyer-loading';
    loading.appendChild(document.createTextNode("Loading..."));
    document.body.appendChild(loading);
    
    var HOSTNAME   = 'sean.anasta.si'
    var PREFIX     = '/projects/dom-destroyer'
    var JQUERY     = 'http://' + HOSTNAME + '/js/jquery-1.4.2-GM.min.js';
    var STYLESHEET = 'http://' + HOSTNAME + PREFIX + '/dom-destroyer.css';

    var GRAPHICS     = 'http://' + HOSTNAME + PREFIX + '/graphics';
    var IMAGES = {
        ship_noflame: GRAPHICS + '/ship-noflame.png',
        ship_flame:   GRAPHICS + '/ship-flame.png',
        missile:      GRAPHICS + '/missile.png',
    };
    
    var COLLIDE_INTERVAL = 100;
    var UPDATE_INTERVAL  = 50;
    var DRAW_INTERVAL    = 50;
    
    var SCOREBOARD  = '<div id="dom-destroyer-scoreboard">' +
                        '<h3>Your Score</h3>' +
                        '<div id="dom-destroyer-score">0</div>' +
                      '</div>';
    var GAME_CANVAS = '<canvas id="dom-destroyer-canvas">' +
                        "Your browser doesn't support the HTML5 canvas tag." +
                      '</canvas>';
    
    
    var KEY = { LEFT: 37, UP: 38, RIGHT: 39, DOWN: 40, SPACE: 32, NONE: 0 };
    var key = {up: false, spaceReleased: false, down: false, left: false, right: false};
    
    // Game elements.
    var ship      = null;
    var fragments = {};
    var score     = 0;
    var canvas    = null;
    
    // In case we're not in Chrome/Firefox w/ Firebug or Firebug is closed.
    if (window.console === undefined) { window.console = {log:function(){}}; }

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // UTILITY FUNCTIONS
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

    /**
     * Trim whitespace from both ends of a string.
     */
    String.prototype.trim = function () {
        return this.replace(/^\s*/, "").replace(/\s*$/, "");
    };

    /**
     * Returns true if this string is empty or composed entirely of whitespace.
     */
    String.prototype.empty = function () {
        return /^\s*$/.test(this || ' ');
    };

    /**
     * Return the width of this string in the given font.
     * @font
     */
    String.prototype.width = function (font) {
        var ctx = document.getElementById('dom-destroyer-canvas').getContext('2d');
        ctx.save();
        ctx.font = font;
        var metrics = ctx.measureText(this);
        ctx.restore();
        return metrics.width;
    };

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // RESOURCE LOADING
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    /**
     * Require an external javascript file.  Used when in bookmarklet mode.
     * @script   URL of a javascript file.
     * @callback A function to call when finished.
     */
    function require(script, callback) {
        var node = document.createElement("script");
        node.src = script;
        document.body.appendChild(node);
        var wait = setInterval(function () {
            if (window.jQuery != undefined) {
                clearInterval(wait);
                callback();
            }
        }, 100);
    }
    
    /**
     * Load an external CSS stylesheet.
     * @stylesheet URL of a CSS stylesheet.
     */
    function loadStylesheet(stylesheet) {
        var link  = document.createElement("link");
        link.setAttribute('rel',  'stylesheet');
        link.setAttribute('type', 'text/css');
        link.setAttribute('href', stylesheet);
        document.getElementsByTagName('head')[0].appendChild(link);
    }
    
    /**
     * Preload IMAGES.
     * @callback A function to call when finished.
     */
    function loadImages(callback) {
        var unfinished = 0;
        jQuery.each(IMAGES, function(key) {
            var image   = new Image();
            image.onload = function () {
                unfinished--;
                IMAGES[key] = this;
                if (unfinished === 0)
                    callback();
            };
            unfinished++;
            image.src = IMAGES[key];
        });
    }

    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // APPLICATION ENTRY POINT
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    function main () {
        obstacles = parseDocument();
        
        $(document.body).append(GAME_CANVAS);
        canvas = document.getElementById('dom-destroyer-canvas');
        
        function resizeCanvas() {
            canvas.setAttribute('width', window.outerWidth);
            canvas.setAttribute('height', window.outerHeight); }
        $(window).resize(resizeCanvas);
        resizeCanvas();
        
        $(window).keydown (keyDown);
        $(window).keyup   (keyUp);
        $(window).unload  (saveScore);
        
        $("#dom-destroyer-loading").hide();
        $(document.body).append(SCOREBOARD);
        $("#dom-destroyer-scoreboard").show();
        
        ship     = new Ship(700, 70);
        
        setInterval(draw,    DRAW_INTERVAL);
        setInterval(collide, COLLIDE_INTERVAL);
    }
    
    if (window.GM_getResourceURL != undefined)
        loadStylesheet(GM_getResourceURL('stylesheet'));
    else
        loadStylesheet(STYLESHEET);
    
    if (window.jQuery == undefined)
        require (JQUERY, function() { loadImages(main); });
    else
        loadImages (main);

    /**
     * Parse the document, assigning classes 'dom-destroyer-word' and
     * 'dom-destroyer-image' to obstacles and building our collision lookup
     * array.
     * @return The collision lookup array.
     */
    function parseDocument () {
        var obstacles = [];
        
        $('*:visible').contents().filter(function() {
            return this.nodeType == Node.TEXT_NODE
                && !this.nodeValue.empty();
        }).each(function() {
            var words = jQuery.map(this.nodeValue.split(/ +/), function(word) {
                if (word.empty()) return word;
                else return '<span class="dom-destroyer-word">' + word + '</span>';
            }).join(' ');
            $(this).replaceWith(words);
        });
        
        $('img,object,embed').addClass('dom-destroyer-image');
        
        $('.dom-destroyer-word,.dom-destroyer-image').each(function() {
            var obstacle = $(this);
            var offset   = obstacle.offset();
            var minx     = Math.floor(offset.left / 10);
            var maxx     = Math.floor((offset.left + obstacle.width()) / 10);
            
            for (var x = minx; x < maxx; x++) {
                if (obstacles[x] == undefined) obstacles[x] = [];
                var miny = Math.floor(offset.top / 10);
                var maxy = Math.floor((offset.top + obstacle.height()) / 10);
                
                for (var y = miny; y < maxy; y++) {
                    obstacles[x][y] = obstacle;
                }
            }
        });
        
        return obstacles;
    }
    
    /**
     * Do collision detection for ship, missiles and obstacles.
     */
    function collide () {
        var obstacle = ship.intersect(obstacles);
        if (obstacle != undefined) {
            ship.explode();
             if (obstacle.hasClass('dom-destroyer-image'))
                explodeImage(obstacle);
            else
                explodeText(obstacle);
        }
        
        jQuery.each(ship.missiles, function() {
            var obstacle = this.intersect(obstacles);
            if (obstacle != undefined) {
                this.explode();
                if (obstacle.hasClass('dom-destroyer-image'))
                    explodeImage(obstacle);
                else {
                    explodeText(obstacle);
                    increaseScore(obstacle.text());
                }
            }
        });
    }
    
    /**
     * Render ship, missiles and fragments.
     */
    function draw () {
        var ctx = document.getElementById('dom-destroyer-canvas').getContext('2d');
        
        // Clear previous location.
        jQuery.each(ship.missiles, function (id, missile) {
            missile.clearPrev(ctx);
            missile.setPrev();
        });
        
        ship.clearPrev(ctx);
        ship.setPrev();
        
        jQuery.each(fragments, function (id, fragment) {
            fragment.clearPrev(ctx);
            fragment.setPrev();
        });

        // Redraw.
        jQuery.each(ship.missiles, function (id, missile) {
            missile.draw(ctx); 
        });

        ship.draw(ctx);
        
        jQuery.each(fragments, function (id, fragment) {
            fragment.draw(ctx);
        });
        
        // Do auto scrolling.
        
        var dx = (ship.x - window.outerWidth / 2);
        var dy = (ship.y - window.outerHeight / 2);
        
        window.scrollBy(dx, dy);
        
        if ($(window).scrollLeft() + ship.x > window.outerWidth / 2 
        && $(window).scrollLeft() + ship.x < $(document).width() - window.outerWidth / 2) {
            ship.x -= dx;
            jQuery.each(fragments, function (id, fragment) {
                fragment.x -= dx;
            });
            
            jQuery.each(ship.missiles, function (id, missile) {
                missile.x -= dx;
            });
        }
        
        if ($(window).scrollTop() + ship.y > window.outerHeight / 2 
        && $(window).scrollTop() + ship.y < $(document).height() - window.outerHeight / 2) {
            ship.y -= dy;
            jQuery.each(fragments, function (id, fragment) {
                fragment.y -= dy;
            });
            
            jQuery.each(ship.missiles, function (id, missile) {
                missile.y -= dy;
            });
        }
    }
    
    var characterScores = {
        'E': 1,    'T': 2,     'A': 2,    'O': 3,     'I': 3,    'N': 3,    'H': 4,
        'S': 4,    'R': 4,     'D': 5,    'L': 5,     'U': 7,    'M': 8,    'C': 8,
        'W': 8,    'G': 8,     'F': 8,    'Y': 8,     'P': 8,    ',': 8,    '.': 8,
        'B': 8,    'K': 10,    'V': 10,   '"': 10,    "'": 10,   '-': 20,   '?': 30,
        'X': 30,   'J': 30,    ';': 40,   '!': 40,    'Q': 40,   'Z': 50,   ':': 60,
        '1': 70,   '—': 80,    '0': 90,   ')': 90,    '*': 90,   '(': 90,   '2': 90,
        '’': 90,   '`': 90,    '“': 90,   '”': 90,    '3': 90,   '9': 90,   '5': 90,
        '4': 90,   '8': 100,   '7': 100,  '6': 100,   '/': 100,  '_': 100,  '[': 100,
        '»': 100,  ']': 100,   '«': 100,  '=': 1000,  '´': 1000, ' ': 1000, '>': 1000,
        '~': 1000, '<': 1000,  '#': 1000, '·': 1000,  '‘': 1000, '&': 1000, '{': 1000,
        '}': 1000, '•': 1000,  '^': 1000, '|': 1000, '\\': 1000, '@': 1000, '%': 1000,
        '$': 1000, 'Ñ': 10000, "\n": 0
    };

    /**
     * Increase score counter for the letters in the given text.
     */
    function increaseScore(text) {
        for (var i = 0; i < text.length; i++) {
            var points = characterScores[text.charAt(i).toUpperCase()];
            if (points === undefined) {
                console.log(text.charAt(i) + ' undefined');
                points = 10000;
            }
            score += (points - 0);
        }
        $('#dom-destroyer-score').text(score);
    }
    
    /**
     * Save score to the server.
     */
    function saveScore() {
        
    }
    
    /**
     * Turn the given image into a set of fragments.
     */
    function explodeImage(imageNode) {
        console.log("Asplode image.");
        var url    = imageNode.src;
        
    }
    
    /**
     * Turn the give text into a set of fragments.
     */
    function explodeText(textNode) {
        console.log("Asplode text.");
        var str    = textNode.text();
        var offset = textNode.offset();
        var font   = textNode.css('font-style') + " " + textNode.css('font-variant') + 
               " " + textNode.css('font-weight') + " " + textNode.css('font-size') + 
               " " + textNode.css('font-family');
        var color = textNode.css('color');
        var x = offset.left;
        var y = offset.top - $(window).scrollTop();
        var height = textNode.height();
        
        jQuery.each(str.split(''), function() {
            // letter, font, x, y, theta, dx, dy, dtheta
            var width = this.width(font);
            
            var dx = Math.random() * 5 - 2.5;
            var dy = Math.random() * 5 - 2.5;
            var dtheta = Math.random() * 2 - 1;
            
            var fragment = new TextFragment(this, font, color, width, height, x, y, 0, dx, dy, dtheta);
            fragments[fragment.id] = fragment;
            x += width;
        });

        textNode.css('visibility', 'hidden');
    }
    
    
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // SPRITE BASE CLASS
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Sprite.nsprites = 0;
    function Sprite (x,y,width,height,theta,image) {
        this.x = x;
        this.y = y;
        this.width = width;
        this.height = height;
        this.theta = theta;
        this.image = image;
        
        // "Previous values", used for clearing the canvas.
        this.prevX = x;
        this.prevY = y;
        this.prevTheta = theta;
        this.id = Sprite.nsprites++;
    }
    
    /**
     * Find the first obstacle in the given 2D array that intersects with this
     * sprite.
     * @obstacles     A 2D obstacle array. Obstacle at [i][j] is located within
     *                the 10x10 area from i*10,j*10 to (i+1)*10-1,(j+1)*10-1.
     */
    Sprite.prototype.intersect = function (obstacles) {
        function obstacleAt(x,y) {
            if (obstacles[Math.floor(x/10)] != undefined) {
                var o = obstacles[Math.floor(x/10)][Math.floor((y + $(window).scrollTop())/10)];
                if ($(o).css('visibility') !== 'hidden')
                    return o;
            }
            return undefined;
        }

        var radius   = Math.max(this.width, this.height);
        var obstacle = undefined;
        for (var ix = -this.width/2; ix < this.width/2; ix++) {
            for (var iy = -this.height/2; iy < this.height/2; iy++) {
                obstacle = obstacle || obstacleAt(this.x + ix, this.y + iy);
            }
        }
        return obstacle;
    };
    
    /**
     * Save the current location of this sprite.
     */
    Sprite.prototype.setPrev     = function(ctx) {
        this.prevX     = this.x;
        this.prevY     = this.y;
        this.prevTheta = this.theta;
    };
    
    /**
     * Clear the last saved location of this sprite.
     * @ctx The canvas context to use.
     */
    Sprite.prototype.clearPrev   = function(ctx) {
        ctx = ctx || canvas.getContext('2d');
        
        // Clear previous draw.
        ctx.save();
        ctx.translate(this.prevX, this.prevY);
        ctx.rotate(Math.PI / 180.0 * this.prevTheta);
        ctx.translate(-this.width / 2, -this.height/2);
        ctx.clearRect(-5, -5, this.width+10, this.height+10);
        ctx.restore();
    };
    
    /**
     * Draw this sprite to the canvas.
     * @ctx The canvas context to use.
     */
    Sprite.prototype.draw        = function(ctx) {
        ctx = ctx || canvas.getContext('2d');
        // Draw current position.
        ctx.save();
        ctx.translate(this.x, this.y);                  // Translate x,y to 0
        ctx.rotate(Math.PI / 180.0 * this.theta);       // Rotate around origin by theta
        ctx.translate(-this.width / 2, -this.height/2); // Center ship will be centered on x,y
        ctx.drawImage(this.image, 0, 0, this.width, this.height);
        ctx.restore();
    };
    
    /**
     * Tell this sprite to explode.
     */
    Sprite.prototype.explode = function() {
        
    };
    
    
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // SHIP
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Ship.prototype = new Sprite();
    Ship.prototype.parent = Sprite;
    Ship.prototype.constructor = Ship;

    // Constants
    Ship.prototype.D2THRUST   = 1;
    Ship.prototype.MAX_THRUST = 10;
    Ship.prototype.DTHETA     = 10;
    /**
     * Construct a new ship at (x,y)
     */
    function Ship(x, y) {
        Sprite.call(this, x, y, 28, 40, 0, IMAGES.ship_noflame);
        this.dx     = 0;
        this.dy     = 0;
        this.thrust = 0;
        this.missiles = {};
        
        var ship = this;
        this.interval = setInterval(function() { ship.update(); }, UPDATE_INTERVAL);
    }

    /**
     * Update this ship's position and velocity.
     */
    Ship.prototype.update = function () {
        
        this.x += this.dx;
        this.y += this.dy;
        
        if (key.up) {
            // Thruster on, increase thrust by D2THRUST up to MAX_THRUST.
            this.thrust = Math.min(this.thrust + this.D2THRUST, this.MAX_THRUST);
            this.image = IMAGES.ship_flame;
        } else {
            // Else decrease by D2THRUST/2 until 0.
            this.thrust = Math.max(this.thrust - this.D2THRUST/2, 0);
            this.image = IMAGES.ship_noflame;
        }
        
        this.dy = Math.sin((this.theta - 90) * Math.PI / 180.0) * this.thrust;
        this.dx = Math.cos((this.theta - 90) * Math.PI / 180.0) * this.thrust;
        
        if (key.left) {
            this.theta = (this.theta - this.DTHETA) % 360;
        }
        
        if (key.right) {
            this.theta = (this.theta + this.DTHETA) % 360;
        }
        
        if (key.spaceReleased) {
            key.spaceReleased = false;
            
            var ix  = this.x + Math.cos((this.theta - 90) * Math.PI / 180.0) * (this.width / 2 + 20);
            var iy  = this.y + Math.sin((this.theta - 90) * Math.PI / 180.0) * (this.height / 2 + 20);
            
            var idx = this.dx + Math.cos((this.theta - 90) * Math.PI / 180.0) * 10;
            var idy = this.dy + Math.sin((this.theta - 90) * Math.PI / 180.0) * 10;
            
            var missiles = this.missiles;
            var missile = new Missile(ix, iy, this.theta, idx, idy);
            missile.onDestroy = function onDestroy() { delete missiles[missile.id]; };
            this.missiles[missile.id] = missile;
        }
    };


    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // MISSILE
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Missile.prototype = new Sprite();
    Missile.prototype.parent = Sprite;
    Missile.prototype.constructor = Missile;
    /**
     * Construct a new missile at (x,y) with angle theta and velocity (dx,dy).
     */
    function Missile (x, y, theta, dx, dy, onDestroy) {
        Sprite.call(this, x, y, 8, 20, theta, IMAGES.missile);
        this.dx = dx;
        this.dy = dy;
        this.onDestroy = onDestroy;
        
        var missile = this;
        this.interval = setInterval(function() { missile.update(); }, UPDATE_INTERVAL);
    }

    /**
     * Update this missile's position and velocity.
     */
    Missile.prototype.update = function () {
        this.x += this.dx;
        this.y += this.dy;
        
        if (this.x > window.outerWidth || this.x < 0 || this.y > window.outerHeight || this.y < 0) {
            this.clearPrev();
            this.onDestroy();
            clearInterval(this.interval);
        }
    };

    /**
     * Destroy this missile.
     */
    Missile.prototype.destroy = function() {
        parent.destroy();
        this.onDestroy();
        clearInterval(this.interval);
    };
    
    TextFragment.id = 0;
    function TextFragment (letter, font, color, width, height, x, y, theta, dx, dy, dtheta) {
        this.letter = letter;
        this.font = font;
        this.color = color;
        this.width = width;
        this.height = height;
        this.x = x;         this.prevX = x;
        this.y = y;         this.prevY = y;
        this.theta = theta; this.prevTheta = theta;
        this.dtheta = dtheta;
        this.dx = dx;
        this.dy = dy;
        this.id = TextFragment.id++;
        
        var fragment = this;
        this.interval = setInterval(function() { fragment.update(); }, UPDATE_INTERVAL);
        setTimeout(function() { fragment.destroy(); }, Math.random() * 3000 + 2000);
    }
    
    TextFragment.prototype.update = function () {
        this.x += this.dx;
        this.y += this.dy;
        this.theta += this.dtheta;
    };
    
    TextFragment.prototype.setPrev = function () {
        this.prevX = this.x;
        this.prevY = this.y;
        this.prevTheta = this.theta;
    };
    
    TextFragment.prototype.clearPrev = function (ctx) {
        ctx = ctx || document.getElementById('dom-destroyer-canvas').getContext('2d');
        ctx.save();
        ctx.translate(this.prevX, this.prevY);
        ctx.rotate(Math.PI / 180.0 * this.prevTheta);
        ctx.translate(-this.width / 2, -this.height/2);
        ctx.clearRect(-5, -10, this.width+10, this.height+20);
        ctx.restore();
    };
    
    TextFragment.prototype.draw = function (ctx) {
        ctx = ctx || document.getElementById('dom-destroyer-canvas').getContext('2d');
        ctx.save();
        ctx.translate(this.x, this.y);
        ctx.rotate(Math.PI / 180.0 * this.theta);
        ctx.translate(-this.width / 2, -this.height/2);
        ctx.font = this.font;
        ctx.fillStyle = this.color;
        //ctx.fillRect(0, 0, this.width, this.height);
        ctx.fillText(this.letter, 0, this.height);
        ctx.restore();
    };
    
    TextFragment.prototype.destroy = function () {
        this.clearPrev();
        clearInterval(this.interval);
        delete fragments[this.id];
    };
   
    
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    // KEY HANDLERS
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    function keyDown (event) {
        switch (event.keyCode) {
        case KEY.LEFT:  key.left  = true; break;
        case KEY.RIGHT: key.right = true; break;
        case KEY.UP:    key.up    = true; break;
        case KEY.DOWN:  key.down  = true; break;
        case KEY.SPACE: break;
        default: return true;
        }
        event.stopPropagation();
        return false;
    }

    function keyUp (event) {
        switch (event.keyCode) {
        case KEY.LEFT:  key.left  = false; break;
        case KEY.RIGHT: key.right = false; break;
        case KEY.UP:    key.up    = false; break;
        case KEY.DOWN:  key.down  = false; break;
        case KEY.SPACE: key.spaceReleased = true; break;
        default: return true;
        }
        event.stopPropagation();
        return false;
    }
    
})();

