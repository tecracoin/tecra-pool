<?php
global $_ravenclient;
try{
    // https://github.com/getsentry/sentry-php/tree/1.10.0
    $_ravenclient = (new Raven_Client('https://5852d40ea0a94278af93f5117cd4db6d@sentry.io/1780966'))->install();
}catch(\Exception $e){
    error_log("Couldnt initialize sentry: ".$e->getMessage());
}
function dd(...$args){
    var_dump($args);
    die();
}
