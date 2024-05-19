const char Web_page[] PROGMEM = R"=====( 
<!DOCTYPE html>
<html>
    <head>
        <title>USBvalve</title>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <style>
            html,body{
                margin: 0;
                padding: 0;
                background-color: #000;
                height:100%;
            }
            header{
                display: block;
                height: 100px;
                text-align: center;
                font-size: 50px;
                color: #fff;
                line-height: 90px;
                text-shadow: #ccc 0 1px 0, #c9c9c9 0 2px 0, #bbb 0 3px 0, #b9b9b9 0 4px 0, #aaa 0 5px 0,rgba(0,0,0,.1) 0 6px 1px, rgba(0,0,0,.1) 0 0 5px, rgba(0,0,0,.3) 0 1px 3px, rgba(0,0,0,.15) 0 3px 5px, rgba(0,0,0,.2) 0 5px 10px, rgba(0,0,0,.2) 0 10px 10px, rgba(0,0,0,.1) 0 20px 20px;

            }
            #content{
                display: block;
                width: 92%;
                height: calc(100% - 120px);
                background-color: #1e1e1e;
                margin: 0 auto;
                color:darkorange;
                font-size: 18px;
                line-height:20px;
                padding: 10px;
                box-sizing: border-box;
                border: thin solid #2f2f2f;
                overflow: auto;
                white-space: pre-line;
            }
        </style>
    </head>
    <body>
        <header>USBvalve</header>
        <div id="content">
        </div>
        <script>
       setInterval(function() {getValveData();}, 500); // Call the update function every set interval e.g. 1000mS or 1-sec
  
       function getValveData() {
          var xhttp = new XMLHttpRequest();
          xhttp.onreadystatechange = function() {
          if (this.readyState == 4 && this.status == 200) {
            document.getElementById("content").innerHTML += this.responseText.replace(/(?:\r\n|\r|\n)/g, '<br>').replace('<br><br>','<br>');
            var objDiv = document.getElementById("content");
            objDiv.scrollTop = objDiv.scrollHeight;
          }
        };
        xhttp.open("GET", "valveout", true);
        xhttp.send();
      }
    </script>
    
    </body>
</html>
)=====";

const char config_portal[] PROGMEM = R"=====( 
<!DOCTYPE html>
<html>
    <head>
        <title>USBvalve - Config</title>
        <meta charset="UTF-8">
        <meta name="viewport" content="width=device-width, initial-scale=1.0">
        <style>
            html,body{
                margin: 0;
                padding: 0;
                background-color: #000;
                height:100%;
            }
            header{
                display: block;
                height: 100px;
                text-align: center;
                font-size: 50px;
                color: #fff;
                line-height: 90px;
                text-shadow: #ccc 0 1px 0, #c9c9c9 0 2px 0, #bbb 0 3px 0, #b9b9b9 0 4px 0, #aaa 0 5px 0,rgba(0,0,0,.1) 0 6px 1px, rgba(0,0,0,.1) 0 0 5px, rgba(0,0,0,.3) 0 1px 3px, rgba(0,0,0,.15) 0 3px 5px, rgba(0,0,0,.2) 0 5px 10px, rgba(0,0,0,.2) 0 10px 10px, rgba(0,0,0,.1) 0 20px 20px;

            }
            #content{
                display: block;
                width: 92%;
                background-color: #1e1e1e;
                margin: 0 auto;
                color:darkorange;
                font-size: 18px;
                padding: 10px;
                box-sizing: border-box;
                border: thin solid #2f2f2f;
            }
            input[type=submit],.btn {
                border: none;
                color: #FEFCFB;
                background-color: #034078;
                padding: 15px 15px;
                text-align: center;
                text-decoration: none;
                display: inline-block;
                font-size: 16px;
                width: 100px;
                margin-right: 10px;
                border-radius: 4px;
                transition-duration: 0.4s;
            }

            input[type=submit]:hover{
                background-color: #1282A2;
            }
            .btn {
                background-color: #661212;
                cursor: pointer;
                margin-left: 50px;
            }
            .btn:hover {
                background-color: #aa1212;
            }
            input[type=text], input[type=password], input[type=number], select {
                width: 50%;
                padding: 12px 20px;
                margin: 18px;
                display: inline-block;
                border: 1px solid #ccc;
                border-radius: 4px;
                box-sizing: border-box;
            }
            input[type=checkbox]{
                margin-left: 18px;
            }
            label {
                font-size: 1.2rem;
                display: inline-block;
                width: 160px;
                margin-left: 10px;
            }
            .blocktop{
                display: block;
                width: 100%;
                font-size: 1.4rem;
                color: #fff;
                font-weight: bold;
                background-color: #363636;
                line-height: 2rem;
                text-align: center;
            }
            .block{
                display: block;
                width: 98%;
                border: thin solid #363636;
                margin: 0 auto;
            }
            .btnblock{
                display: block;
                width: 98%;
                text-align: center;
            }
        </style>
    </head>
    <body>
        <header>USBvalve - Config</header>
        <div id="content">
            <br>
            <form action="/" method="POST">
                <div class="block">
                    <div class="blocktop">WiFi - Settings</div>
                    <div>
                        <br>
                        <label for="ssid">SSID</label>
                        <input type="text" id ="ssid" name="ssid"><br>
                        <label for="pass">Password</label>
                        <input type="password" id ="pass" name="pass"><br>
                        <label for="pvis">Show Password</label>
                        <input type="checkbox" id ="pvis" name="pvis"  onclick="pasvis()"><br><br>
                    </div>
                </div>
                <br>
                <div class="block">
                    <div class="blocktop">Static IP</div>
                    <div>
                        <br>
                        <label for="ip">IP Address</label>
                        <input type="text" id ="ip" name="ip" placeholder="leave blank for DHCP" value=""><br>
                        <label for="gateway">Gateway Address</label>
                        <input type="text" id ="gateway" name="gateway" placeholder="leave blank for DHCP" value=""><br>
                        <br>
                    </div>
                </div>
                <br>
                <div class="btnblock">
                    <input type ="submit" value ="Save">
                    <span class="btn" onclick="reset();">Reset</span>
                </div>
            </form>
            <br>
        </div>
        <form  action="/reset" method="POST" name="resetit" id="resetit">
            <input type="hidden" value="reset"> 
        </form>
        <script>
            function pasvis() {
                var x = document.getElementById("pass");
                if (x.type === "password") {
                    x.type = "text";
                } else {
                    x.type = "password";
                }
            }
            function reset() {
                document.getElementById('resetit').submit();
            }
        </script>

    </body>
</html>
)=====";
