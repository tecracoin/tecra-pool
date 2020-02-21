<?php

require('misc.php');
echo <<<END

<!doctype html>
<!--[if IE 7 ]>		 <html class="no-js ie ie7 lte7 lte8 lte9" lang="en-US"> <![endif]-->
<!--[if IE 8 ]>		 <html class="no-js ie ie8 lte8 lte9" lang="en-US"> <![endif]-->
<!--[if IE 9 ]>		 <html class="no-js ie ie9 lte9>" lang="en-US"> <![endif]-->
<!--[if (gt IE 9)|!(IE)]><!--> <html class="no-js" lang="en-US"> <!--<![endif]-->

<head>

<meta charset="utf-8">
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta http-equiv="X-UA-Compatible" content="IE=edge,chrome=1">
<meta name="viewport" content="width=device-width, initial-scale=0.5">
<meta name="description" content="Yii mining pools for alternative crypto currencies">
<meta name="keywords" content="anonymous,mtp,mining,pool,maxcoin,bitcoin,altcoin,auto,switch,exchange,profit,decred,scrypt,x11,x13,x14,x15,lbry,lyra2re,neoscrypt,sha256,quark,skein2">
<!--link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/4.7.0/css/font-awesome.min.css"-->
<link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/5.12.0/css/all.min.css">
END;

$pageTitle = empty($this->pageTitle) ? YAAMP_SITE_NAME : YAAMP_SITE_NAME." - ".$this->pageTitle;

echo '<title>'.$pageTitle.'</title>';

echo CHtml::cssFile("/extensions/jquery/themes/ui-lightness/jquery-ui.css");
echo CHtml::cssFile('/yaamp/ui/css/main.css');
echo CHtml::cssFile('/yaamp/ui/css/table.css');

//echo CHtml::scriptFile('/extensions/jquery/js/jquery-1.8.3-dev.js');
//echo CHtml::scriptFile('/extensions/jquery/js/jquery-ui-1.9.1.custom.min.js');

$cs = app()->getClientScript();
$cs->registerCoreScript('jquery.ui');
//$cs->registerScriptFile('/yaamp/ui/js/jquery.tablesorter.js', CClientScript::POS_END);

echo CHtml::scriptFile('/yaamp/ui/js/jquery.tablesorter.js');

// if(!controller()->admin)
// echo <<<end
// <script>
// (function(i,s,o,g,r,a,m){i['GoogleAnalyticsObject']=r;i[r]=i[r]||function(){
// (i[r].q=i[r].q||[]).push(arguments)},i[r].l=1*new Date();a=s.createElement(o),
// m=s.getElementsByTagName(o)[0];a.async=1;a.src=g;m.parentNode.insertBefore(a,m)
// })(window,document,'script','//www.google-analytics.com/analytics.js','ga');

// ga('create', 'UA-58136019-1', 'auto');
// ga('send', 'pageview');

// $(document).ajaxSuccess(function(){ga('send', 'pageview');});

// </script>
// end;

echo "</head>";

///////////////////////////////////////////////////////////////

echo '<body class="page">';
echo '<a href="/site/mainbtc" style="display: none;">main</a>';

showPageHeader();
showPageContent($content);
showPageFooter();

echo "</body></html>";
return;

/////////////////////////////////////////////////////////////////////

function showItemHeader($selected, $url, $name)
{
	if($selected) $selected_text = "class='selected'";
	else $selected_text = '';
	echo "<a href='$url' $selected_text >$name</a>";
//	echo "<span class='menu'><a $selected_text href='$url'>$name</a></span>";
//	echo "&nbsp;";

}

function showPageHeader()
{

	
	echo '<div class="tabmenu-out">';
	echo '<div class="tabmenu-inner">';

	echo '<div style="display:inline"><span><img style="vertical-align:middle" height="40px" src="/images/base/TCR/logo@2x.png"></span></div>';
	echo '<div style="display:inline" class="topnav" id="myTopnav">';
//	echo '&nbsp;&nbsp;<a href="/">'.YAAMP_SITE_NAME.'</a>';
	echo '&nbsp;&nbsp;';

	echo '<span style="float:right   ;vertical-align:middle">';
	 echo '<a href="javascript:void(0);" style="font-size:2em;float:right   ;" class="icon menuresponsive" onclick="ResponsiveFunction()">Menu &#9776;</a>';
	 echo '</span>';
	



	$action = controller()->action->id;
	$wallet = user()->getState('yaamp-wallet');
	$ad = isset($_GET['address']);

	showItemHeader(controller()->id=='site' && $action=='index' && !$ad, '/', 'Home');
	showItemHeader($action=='mining', '/site/mining', 'Pool');
	showItemHeader(controller()->id=='site'&&($action=='index' || $action=='wallet') && $ad, "/?address=$wallet", 'Wallet');
	showItemHeader(controller()->id=='stats', '/stats', 'Graphs');
//	showItemHeader($action=='miners', '/site/miners', 'Miners');
	if (YIIMP_PUBLIC_EXPLORER)
		showItemHeader(controller()->id=='explorer', '/explorer', 'Explorers');

	if (YIIMP_PUBLIC_BENCHMARK)
		showItemHeader(controller()->id=='bench', '/bench', 'Benchs');

	if (YAAMP_RENTAL)
		showItemHeader(controller()->id=='renting', '/renting', 'Rental');

	if(controller()->admin)
	{
echo '<div class="dropdown">';
echo '<button class="dropbtn">Admin<i /*class="fa fa-caret-down"*/></i></button>';
echo	'<div class="dropdown-content">';		

		if (isAdminIP($_SERVER['REMOTE_ADDR']) === false)
			debuglog("admin {$_SERVER['REMOTE_ADDR']}");

		showItemHeader(controller()->id=='coin', '/coin', 'Coins');
		showItemHeader($action=='common', '/site/common', 'Dashboard');
		showItemHeader(controller()->id=='site'&&$action=='admin', "/site/admin", 'Wallets');

		if (YAAMP_RENTAL)
			showItemHeader(controller()->id=='renting' && $action=='admin', '/renting/admin', 'Jobs');

		if (YAAMP_ALLOW_EXCHANGE)
			showItemHeader(controller()->id=='trading', '/trading', 'Trading');

		if (YAAMP_USE_NICEHASH_API)
			showItemHeader(controller()->id=='nicehash', '/nicehash', 'Nicehash');

echo '</div></div>';
	}

	/*	<div class="theme-switch-wrapper"> */
   
echo <<<TRUC
<input class="container_toggle" style="visibility: hidden" type="checkbox" id="switch" name="mode">

<label for="switch" id="nightswitch"><i style="color:#efef51"  class="fas fa-moon  fa-1x"></i></label>

<script type="text/javascript">
var checkbox = document.querySelector('input[name=mode]');

	   checkbox.addEventListener('change', function() {
		   if(this.checked) {
			   trans() 
			   document.documentElement.setAttribute('data-theme', 'dark')
			   localStorage.setItem('theme', 'dark');
			   $("#nightswitch").html('<i style="color:#efef51" class="fas fa-sun  fa-1x"></i>');
		   } else {
			   trans() 
			   document.documentElement.setAttribute('data-theme', 'light')
			   localStorage.setItem('theme', 'light');
			   $("#nightswitch").html('<i style="color:#efef51"  class="fas fa-moon  fa-1x"></i>');
		   }
	   })

	   let trans = () => {
		   document.documentElement.classList.add('transition');
		   window.setTimeout(() => {
			   document.documentElement.classList.remove('transition');
		   }, 1000)
	   }

	   const currentTheme = localStorage.getItem('theme') ? localStorage.getItem('theme') : 'light';

	   if (currentTheme) {
		   document.documentElement.setAttribute('data-theme', currentTheme);
	   
		   if (currentTheme === 'dark') {
			checkbox.checked = true;
		   }
	   }	   
</script>

TRUC;
/*	
	echo '<span style="float:right   ;vertical-align:middle">';
	 echo '<a href="javascript:void(0);" style="font-size:2em;float:right   ;" class="icon menuresponsive" onclick="ResponsiveFunction()">Menu &#9776;</a>';
	 echo '</span>';
*/	 
	echo '</div>';

	$mining = getdbosql('db_mining');
	$nextpayment = date('H:i T', $mining->last_payout+YAAMP_PAYMENTS_FREQ);
	$eta = ($mining->last_payout+YAAMP_PAYMENTS_FREQ) - time();
	$eta_mn = 'in '.round($eta / 60).' minutes';

	echo '<div class="payoutresponsive" style=" display:inline; /*float:right*/"><span id="nextpayout" style="font-size: .8em; display:inline-block" title="'.$eta_mn.'">Next Payout: '.$nextpayment.'</span></div>';
	echo '</div>';

	echo "</div>";

echo <<<TRUC
	<script>
	function ResponsiveFunction() {
	  var x = document.getElementById("myTopnav");
	  if (x.className === "topnav") {
		x.className += " responsive";
	  } else {
		x.className = "topnav";
	  }
	}
	</script>
	
TRUC;

}

function showPageFooter()
{
	echo '<div class="footer">';
	$year = date("Y", time());

echo "<p>&copy; $year ".YAAMP_SITE_NAME.' - '.
		'<a href="https://tecracoin.io/">Tecracoin</a></p>';

	echo '</div><!-- footer -->';
}


