<?php
if (session_id() === '') {
    @session_start();
}

if (!function_exists('session_register')) {
    function session_register($name) {
        if (isset($GLOBALS[$name])) {
            $_SESSION[$name] = $GLOBALS[$name];
        } else {
            $_SESSION[$name] = null;
        }
        return true;
    }
}

if (!function_exists('session_unregister')) {
    function session_unregister($name) {
        unset($_SESSION[$name]);
        return true;
    }
}

if (!function_exists('session_is_registered')) {
    function session_is_registered($name) {
        return isset($_SESSION[$name]);
    }
}

if (!function_exists('mysql_escape_string')) {
    function mysql_escape_string($string) {
        return addslashes($string);
    }
}
