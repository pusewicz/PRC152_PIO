const char *html_RCU=
R"(
<!-- <!DOCTYPE html> -->
<html>

<head>
    <style>
        .decorateLine{
            padding: 20px;
            text-align: center;
        }
        .decorate{
            background-color: rgb(43, 56, 30);
            width: 40px;
            height: 10px;
            border-radius: 10px;
            margin: 10px auto;
            display: inline-block;
        }
        /* 去除顶部白边 */
        .body {
            margin: 0 auto;
            padding: 0;
            text-align: center;
            background-color: rgb(53, 70, 38);

        }

        .allBackground {
            margin: 0 auto;
            width: 350px;
            background-color: rgb(43, 56, 30);
            border-radius: 10px;
            border: 1px solid rgb(95, 95, 95);
        }

        .screen {
            width: 330px;
            height: 80px;
            border-radius: 20px;
            border-width: 5px;
            border-style: solid;
            border-color: black;
            background-color: #f3f701;
            /* 整个居中 */
            margin: 0 auto;
            /* 所有子元素垂直居中 */
            display: flex;
            align-items: center;
        }

        .FCS {
            padding: 15px;
            padding-top: 30px;
            text-align: center;
            font-size: larger;
            font-weight: bolder;
            font-style: italic;
            color: white;
            margin: 0 auto;
        }

        .textVar {
            font-weight: bolder;
            color: brown;
            border: 0;
            width: 70px;
            background-color: #f3f701;
        }

        .textLabel {
            font-weight: bolder;
            font-size: 14px;
        }

        .divBtn {
            width: fit-content;
            margin: 0 auto;
            display: flex;
            align-items: center;
            /* 底部拉长 */
            padding-bottom: 20px;
        }

        .btn {
            border-radius: 10px;
            border-color: white;
            background-color: black;
            color: white;
            width: 70px;
            height: 40px;
            font-size: 20px;
            font-weight: bolder;
            margin-left: 10px;
            margin-top: 10px
        }

        /* 与textVar相比缩短宽度 */
        .volVar {
            font-weight: bolder;
            color: brown;
            border: 0;
            width: 20px;
            background-color: #f3f701;
        }
    </style>
    <meta charset='utf-8'>
    <title>FCS PRC152 RT-Control</title>
</head>

<!-- <div style='padding:20px;'></div> -->
<body class='body' onload='getNow()'>

    <div class='decorateLine'>
        <div class='decorate'></div>
        <div class='decorate'></div>
        <div class='decorate'></div>
        <div class='decorate'></div>
        <div class='decorate'></div>
    </div>


    <div class='allBackground'>
        
        <div style='padding:20px; color: white;'>
            WIDEBAND NETWORKING
        </div>

        <div class='screen' >
            <table border='0px'>
                <tr>
                    <td class='textLabel' align='right'>
                        Chan :
                    </td>
                    <td>
                        <input type='text' name='currentChan' id='currentChan' class='textVar' disabled='true'
                            value='001'>
                    </td>

                    <td class='textLabel' align='right'>
                        Name :
                    </td>
                    <td>
                        <input type='text' name='chan_nn' id='chan_nn' class='textVar' disabled='true' value='CH-01'>
                    </td>
                </tr>
                <tr>
                    <td class='textLabel' align='right'>
                        Rx :
                    </td>
                    <td>
                        <input type='text' name='rx_freq' id='rx_freq' class='textVar' disabled='true' value='145.55000'>
                    </td>
                    <td class='textLabel' align='right'>
                        Rs :
                    </td>
                    <td>
                        <input type='text' name='rs' id='rs' class='textVar' disabled='true' value='OFF'>
                    </td>
                </tr>
                <tr>
                    <td class='textLabel' align='right'>
                        Tx :
                    </td>
                    <td>
                        <input type='text' name='tx_freq' id='tx_freq' class='textVar' disabled='true' value='145.55000'>
                    </td>
                    <td class='textLabel' align='right'>
                        Ts :
                    </td>
                    <td>
                        <input type='text' name='ts' id='ts' class='textVar' disabled='true' value='OFF'>
                    </td>
                </tr>
            </table>

            <table border='0px'>
                <td class='textLabel'>
                    VOL:
                </td>
                <td>
                    <input type='text' name='volume' id='volume' disabled='true' value='01' class='volVar'>
                </td>
            </table>
        </div>

        <div class='FCS'>
            FCS AN/PRC-152A
        </div>

        <div class='divBtn' style='margin-top: 20px;'>
            <table border='0px' style='margin-left: 0px;'>
                <tr>
                    <td>
                        <input type='button' value='PRE+' class='btn' onclick='PreP()'>
                    </td>
                </tr>
                <tr>
                    <td>
                        <input type='button' value='PRE-' class='btn' onclick='PreN()'>
                    </td>
                </tr>
            </table>

            <table border='0px' style='margin-left: 30px;'>
                <td>
                    <input type='button' value='V/M' class='btn' onclick='SwitchVM()'>
                </td>
            </table>

            <table border='0px' style='margin-left: 30px;'>
                <tr>
                    <td>
                        <input type='button' value='VOL+' class='btn' onclick='VolumeP()'>
                    </td>
                </tr>
                <tr>
                    <td>
                        <input type='button' value='VOL-' class='btn' onclick='VolumeN()'>
                    </td>
                </tr>
            </table>
        </div>

        <!-- <div style='padding-top:20px'></div> -->
    </div>
    
    
</body>

<script>
    //  0:  channel
    //  1:  VHF
    //  2:  UHF
    var nowMode = 0;
    //格式化需要输出的频率
    function formatFreq(val) {
        // alert(typeof(val)+','+val);
        var maxLenth = 9;
        var freq = String(val);
        if (freq.length < maxLenth) {
            if (freq.length < 4) {
                freq = freq + '.' + '0'.repeat(maxLenth - freq.length - 1);
            }
            else {
                freq = freq + '0'.repeat(maxLenth - freq.length);
            }
        }
        return freq;
    }
    function handleAnswer(str) {
        var jsonReturn = window.JSON.parse(str);
        // alert(str);
        // alert(jsonReturn);
        var chan = jsonReturn.chan;
        var rx = jsonReturn.rx_freq;
        var tx = jsonReturn.tx_freq;
        var nn = jsonReturn.chan_nn;
        var rs = jsonReturn.rs;
        var ts = jsonReturn.ts;
        var cf = jsonReturn.cf;
        var vu = jsonReturn.vu;
        var vo = jsonReturn.volume;

        nowMode = cf * (vu + 1);
        if (nowMode == 0)
            document.getElementById('currentChan').value = chan;
        else if (nowMode == 1)
            document.getElementById('currentChan').value = 'VHF';
        else if (nowMode == 2)
            document.getElementById('currentChan').value = 'UHF';

        document.getElementById('chan_nn').value = nn;

        document.getElementById('rx_freq').value = formatFreq(rx);
        document.getElementById('tx_freq').value = formatFreq(tx);
        document.getElementById('rs').value = rs;
        document.getElementById('ts').value = ts;
        document.getElementById('volume').value = vo;
    }
    function handleOperate(mcmd) {
        xmlhttp = new XMLHttpRequest();
        xmlhttp.onreadystatechange = function () {
            if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
                handleAnswer(xmlhttp.responseText);
            }
        }
        xmlhttp.open('POST', mcmd, true);
        xmlhttp.send();
    }
    function getNow() {
        handleOperate('/getNow')
    }
    function PreP() {
        handleOperate('/PRE_P')
    }
    function PreN() {
        handleOperate('/PRE_N')
    }
    function SwitchVM() {
        nowMode = (nowMode + 1) % 3;
        xmlhttp = new XMLHttpRequest();
        xmlhttp.onreadystatechange = function () {
            if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
                handleAnswer(xmlhttp.responseText);
            }
        }
        xmlhttp.open('POST', '/SWITCH?nowMode=' + nowMode, true);
        xmlhttp.send();
    }
    function VolumeP() {
        handleOperate('/VOL_P')
    }
    function VolumeN() {
        handleOperate('/VOL_N')
    }
</script>

</html>

)";