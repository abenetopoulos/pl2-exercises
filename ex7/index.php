<?php

session_start();

$minVal = 1;
$maxVal = 100;

if (isset($_POST['reset']))
    $_SESSION['shouldGenerate'] = True;

if ($_SESSION['shouldGenerate'] || !isset($_SESSION['question'])) {
    $_SESSION['question'] = rand($minVal, $maxVal);
    $_SESSION['shouldGenerate'] = False;
}

function validate($code) {
    $data = trim($code, "\r\n") . "\r\n";

    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, 'http://courses.softlab.ntua.gr/pl2/2016b/exercises/befunge93-api/?restrict');
    curl_setopt($ch, CURLOPT_POST, true);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, true);
    curl_setopt($ch, CURLOPT_HTTPHEADER, array("Content-Type: text/plain"));
    curl_setopt($ch, CURLOPT_POSTFIELDS, $data);
    $output = curl_exec($ch);
    $_SESSION['execOutput'] = $output;
    $response = curl_getinfo($ch, CURLINFO_HTTP_CODE);

    curl_close($ch);

    return $response;
}

?>
<!DOCTYPE html PUBLIC
          "-//W3C//DTD XHTML 1.0 Transitional//EN"
          "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
<title>Funge game!</title>
<style type="text/css">
<!--
body,td,th {
  font-family: Verdana, Arial, Helvetica, sans-serif;
  font-size: x-large;
  color: #CCCCCC;
}

body {
  background-color: #333399;
}

.title {
  font-family: "Courier New", Courier, monospace;
  font-weight: bold;
  font-size: 48px;
  color: #00FF66;
}

.question {color: #FFCC33}
.number {color: #FFFF33}
.md5sum {color: #FFCCFF}
.emph {color: #99ee99}
.alert {color: #ee77aa}

.right {
  color: #33FF66;
  font-weight: bold;
}

.wrong {
  color: #FF3366;
  font-weight: bold;
}

a:link {
  color: #CCFFFF;
}

a:visited {
  color: #CCFFFF;
}

textarea {
  background-color: #eeee66;
  color: #333399;
}

textarea.wide {
  font-family: monospace;
  font-size: x-large;
  color: #333333;
  border: 1px solid black;
  padding: 8px;
}
-->
</style>
</head>
<body>

<h1 class="title">Help!</h1>
<p>I need to find a <a href="http://catseye.tc/view/befunge-93/doc/Befunge-93.markdown">Befunge-93</a> program that outputs the number <span class="question"><?php echo $_SESSION['question']?></span>.</p>
<p>But I also need the program's total area to be as small as possible.<br />
(Don't worry, it doesn't have to be optimal...)</p>
<p>Oh, one more thing: The commands
  <code class="emph">0-9</code>,
  <code class="emph">?</code>,
  <code class="emph">"</code>,
  <code class="emph">p</code>,
  <code class="emph">g</code>,
  <code class="emph">&amp;</code>, and
  <code class="emph">~</code>
  cannot be used.</p>

<p>Enter your program that will print this number!</p>
<form action="<?php echo $_SERVER['PHP_SELF']?>" id="f" name="f" method="post">
<?php
if (isset($_POST['submit'])) {
    $code = $_POST['program'];
    $response = validate($code);

    if ($response == 400) {
        $_SESSION['shouldGenerate'] = False;
        echo '<p class="wrong">Wrong!  :-(</p>
          <p>Your program is not legal:</p>
          <pre>-1</pre>
          <form action="index.php" id="r" name="r" method="post">
          <input type="submit" name="again" id="again" value="Try again!" />
          </form>
          </body>
          </html>';
    } else {
        if ($_SESSION['execOutput'] == $_SESSION['question']) {
            $_SESSION['shouldGenerate'] = True;
            echo '<p class="right">Right!  :-)</p>
                <form action="index.php" id="r" name="r" method="post">
                <input type="submit" name="again" id="again" value="Try again!" />
                </form>
                </body>
                </html>';
        } else {
            $_SESSION['shouldGenerate'] = False;
            echo '<p class="wrong">Wrong!  :-(</p>
                <p>Your program\'s output is wrong:</p>
                <pre>-1</pre>
                <form action="index.php" id="r" name="r" method="post">
                <input type="submit" name="again" id="again" value="Try again!" />
                </form>
                </body>
                </html>';
        }
    }
}else {
    echo '<textarea name="program" id="program" class="wide" rows=20 cols=85></textarea>
        <br />
        <input type="submit" name="submit" id="submit" value="Submit!" />
        <input type="submit" name="reset" id="reset" value="Change number!" />';
}
?>
  </form>
</body>
</html>
