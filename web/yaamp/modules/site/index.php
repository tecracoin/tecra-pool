<?php

$algo = user()->getState('yaamp-algo');

JavascriptFile("/extensions/jqplot/jquery.jqplot.js");
JavascriptFile("/extensions/jqplot/plugins/jqplot.dateAxisRenderer.js");
JavascriptFile("/extensions/jqplot/plugins/jqplot.barRenderer.js");
JavascriptFile("/extensions/jqplot/plugins/jqplot.highlighter.js");
JavascriptFile("/extensions/jqplot/plugins/jqplot.cursor.js");
JavascriptFile('/yaamp/ui/js/auto_refresh.js');

$height = '240px';

$min_payout = floatval(YAAMP_PAYMENTS_MINI);
$min_sunday = $min_payout/10;

$payout_freq = (YAAMP_PAYMENTS_FREQ / 3600)." hours";
?>




<div id='resume_update_button' style='color: #444; background-color: #ffd; border: 1px solid #eea;
	padding: 10px; margin-left: 20px; margin-right: 20px; margin-top: 15px; cursor: pointer; display: none;'
	onclick='auto_page_resume();' align=center>
    <b>Auto refresh is paused - Click to resume</b></div>
    
 <!--  -->

<div class="main-left-box"style='
	 margin:1%'>
<div class="main-left-title">Find miner address:</div>
<div class="main-left-inner">
<form action="/" method="get" style="padding: 10px;">
<input type="text" name="address" class="main-text-input" placeholder="Wallet Address">
<input type="submit" value="Submit" class="main-submit-button" ><br><br>
</div>
</div>
</div>

<div style="text-align:center">
<div style="text-align:center">


<div id='pool_history_results' class="responsive-div-right">
</div>


<div id='pool_current_results' class="responsive-div-left" style="float:left">
</div>


</div>





<div style="text-align:center">


<div id='mining_tecracoin' class="responsive-div-right" style="float:right">
<div class="main-left-box">
<div class="main-left-title">MINING TECRACOIN</div>
<div class="main-left-inner">

<ul>



    <li>
 <!--       <div style="width:100%;overflow:hidden;position:relative;padding:20px;font-size:1.0em;box-sizing:border-box"> -->
           TecraCoin uses as proof of work algorithm a custom implementation of MTP called MTP-TCR. 
           Miners supporting mtp-tcr can be found on <a href="https://github.com/tecracoin">official TecraCoin github<a> .
        <!--/div-->
    </li>
    <li>
        <!--div style="width:100%;overflow:hidden;position:relative;padding:20px;font-size:1.0em;font-weight:bold;text-align:center;box-sizing:border-box"-->
            <a href="https://github.com/tecracoin/ccminer/releases/latest" style="padding: 10px 5px;display:inline-block" target="_blank">
            &gt;&gt;For NVIDIA cards, download the lastest ccminer release &lt;&lt;</a><br/>
            <a href="https://github.com/tecracoin/sgminer/releases/latest" style="padding: 10px 5px;display:inline-block" target="_blank">
            &gt;&gt;For AMD cards, download the lastest sgminer release&lt;&lt;</a>

        <!--/div-->
    </li>
    <li>
        <!--div style="width:100%;overflow:hidden;position:relative;padding:20px;font-size:20px;font-weight:bold;text-align:center;box-sizing:border-box"-->
            Join us on <A HREF="https://t.me/tecracoinio"> telegram</A>
            and/or <A HREF="https://discordapp.com/invite/wA9Cpkd"> discord</a> for support and latest news<BR/><BR/>
            -<i>TecraCoin Team</i>
        <!--/div-->
    </li>

<li>
<p class="main-left-box" style='padding: 3px; font-size: 1.1em; background-color: #ffffee; font-family: monospace;'>
	-a mtp-tcr -o stratum+tcp://<?= YAAMP_STRATUM_URL ?>:&lt;PORT&gt; -u &lt;WALLET_ADDRESS&gt; [-p &lt;OPTIONS&gt;]</p>
</li>

    <?php if (YAAMP_ALLOW_EXCHANGE): ?>
<li>&lt;WALLET_ADDRESS&gt; can be one of any currency we mine or a BTC address.</li>
<?php else: ?>
<li>&lt;WALLET_ADDRESS&gt; must be a valid TCR address. </li>
<?php endif; ?>
<li>As optional password, you can use: 
    <li><b>-p c=&lt;SYMBOL&gt;</b> if yiimp does not set the currency correctly on the Wallet page.</li>
    <li><b>-p d=&lt;difficulty&gt;</b> to start mining with a given difficulty higher than minimum pool default.</li>
    <li><b>-p dm=&lt;difficulty&gt;</b> to impose a minimum difficulty higher than the pool default.</li>
</li>    
<li>See the "Pool Status" area on the right for PORT numbers.</li>

<br>

</ul>
</div></div></div>

<!--  -->
<!--  -->
<div class="responsive-div-left">
<div id='tecra_mining_pool' style="margin-bottom:2%"/*class="responsive-div-left"*/>
<div class="main-left-box">
<div class="main-left-title">TECRACOIN MINING POOL</div>
<div class="main-left-inner">

<ul>

<li>Based on YiiMP pool management solution using Yii Framework.</li>
<li>No registration is required, we do payouts in the currency you mine. Use your wallet address as the username.</li>
<li>&nbsp;</li>
<li>Payouts are made automatically every <?= $payout_freq ?> for all balances above <b><?= $min_payout ?></b>, or <b><?= $min_sunday ?></b> on Sunday.</li>
<li>There is an initial delay before the first payout, please wait at least 6 hours before asking for support.</li>
<li>Blocks are distributed proportionally among valid submitted shares.</li>

<br>

</ul>
</div></div>
<br>
</div>

<div id='links'>
<div class="main-left-box">
<div class="main-left-title">LINKS</div>
<div class="main-left-inner">

<ul>

<!--<li><b>BitcoinTalk</b> - <a href='https://bitcointalk.org/index.php?topic=508786.0' target=_blank >https://bitcointalk.org/index.php?topic=508786.0</a></li>-->
<!--<li><b>IRC</b> - <a href='http://webchat.freenode.net/?channels=#yiimp' target=_blank >http://webchat.freenode.net/?channels=#yiimp</a></li>-->

<li><b>API</b> - <a href='/site/api'>http://<?= YAAMP_SITE_URL ?>/site/api</a></li>
<li><b>Difficulty</b> - <a href='/site/diff'>http://<?= YAAMP_SITE_URL ?>/site/diff</a></li>
<?php if (YIIMP_PUBLIC_BENCHMARK): ?>
<li><b>Benchmarks</b> - <a href='/site/benchmarks'>http://<?= YAAMP_SITE_URL ?>/site/benchmarks</a></li>
<?php endif; ?>

<?php if (YAAMP_ALLOW_EXCHANGE): ?>
<li><b>Algo Switching</b> - <a href='/site/multialgo'>http://<?= YAAMP_SITE_URL ?>/site/multialgo</a></li>
<?php endif; ?>

<br>

</ul>
</div></div><br>
</div>
</div>
</div>
<!--  

<a class="twitter-timeline" href="https://twitter.com/hashtag/YAAMP" data-widget-id="617405893039292417" data-chrome="transparent" height="450px" data-tweet-limit="3" data-aria-polite="polite">Tweets about #YAAMP</a>
<script>!function(d,s,id){var js,fjs=d.getElementsByTagName(s)[0],p=/^http:/.test(d.location)?'http':'https';if(!d.getElementById(id)){js=d.createElement(s);js.id=id;js.src=p+"://platform.twitter.com/widgets.js";
fjs.parentNode.insertBefore(js,fjs);}}(document,"script","twitter-wjs");</script>
 -->

</div>
</div>
<div  class="responsive-div-left"></div>
<div  class="responsive-div-left"></div>
<script>

function page_refresh()
{
	pool_current_refresh();
	pool_history_refresh();
}

function select_algo(algo)
{
	window.location.href = '/site/algo?algo='+algo+'&r=/';
}

////////////////////////////////////////////////////

function pool_current_ready(data)
{
	$('#pool_current_results').html(data);
}

function pool_current_refresh()
{
	var url = "/site/current_results";
	$.get(url, '', pool_current_ready);
}

////////////////////////////////////////////////////

function pool_history_ready(data)
{
	$('#pool_history_results').html(data);
}

function pool_history_refresh()
{
	var url = "/site/history_results";
	$.get(url, '', pool_history_ready);
}

</script>

