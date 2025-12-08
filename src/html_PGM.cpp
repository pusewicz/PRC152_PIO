const char *html_PGM =
R"(
<!-- <!DOCTYPE html> -->
<html lang='en'>

<head>
    <meta charset='UTF-8' />
    <meta name='viewport' content='width=device-width,initial-scale=1.0'>
    <title>ESP32参数配置</title>
    <style type='text/css'>
        .toCenter {
            text-align: center;
        }

        .button {
            width: auto;
        }

        .selectTable {
            width: 100%;
            height: max-content;
            margin: auto;
            height: 35px;
        }

        .inputTable {
            width: 100%;
            border: 0;
            background: transparent;
            text-align: center;
        }

        .labelTable {
            text-align: center;
            /* background: -webkit-linear-gradient(center, blue, green, white); */
            background-color: #293917;
            color: white;
            font: bold;
            height: 35px;
        }

        .titleTable {
            border-top-left-radius: 10px;
            border-top-right-radius: 10px;
            color: white;
            font: bold;
            background-color: #63644a;
        }

        .titleTableBlank {
            border-bottom-left-radius: 10px;
            border-bottom-right-radius: 10px;
            background-color: #63644a;
        }

        .titleTableNoRaidus {
            color: white;
            font: bold;
            background-color: #63644a;
        }

        .table {
            border: 1;
            width: 330px;
            border-collapse: separate;
            border-spacing: 0;
            border-radius: 10px;
            table-layout: fixed;
            white-space: nowrap;
        }
    </style>
</head>

<body onload='getAll()' style='background-color: #252817;'>

    <!-- upgrade:升级 -->
    <div class='toCenter' style='margin-top: 25px;'>
        <form method='POST' action='/update' enctype='multipart/form-data'>
            <input type='file' value='File' name='update' aria-label='file' style='color:#FFFFFF'>
            <input type='submit' value='Update' class='button'>
        </form>
        <hr width='300px' align='center' size='3px' noshade='true' />
    </div>

    <!-- program:编程 -->
    <div class='toCenter' style='margin-top: 25px;'>

        <div style='display:inline-block;'>
            <table border='1' cellpadding='0' class='table'>
                <tr>
                    <td class='titleTable' colspan='2' align='center' height='30'>
                        Channel Setting
                    </td>
                </tr>
                <tr>
                    <td class='labelTable'>PARAMETER</td>
                    <td class='labelTable'>VALUE</td>
                </tr>

                <tr>
                    <td class='labelTable'>CURRENT_CHAN</td>
                    <td> <select name='chan' id='chan' class='selectTable' onchange='getChanParameter(this)'>
                            <option value='0'>VHF</option>
                            <option value='100'>UHF</option>
                            <option value='1'>1</option>
                            <option value='2'>2</option>
                            <option value='3'>3</option>
                            <option value='4'>4</option>
                            <option value='5'>5</option>
                            <option value='6'>6</option>
                            <option value='7'>7</option>
                            <option value='8'>8</option>
                            <option value='9'>9</option>
                            <option value='10'>10</option>
                            <option value='11'>11</option>
                            <option value='12'>12</option>
                            <option value='13'>13</option>
                            <option value='14'>14</option>
                            <option value='15'>15</option>
                            <option value='16'>16</option>
                            <option value='17'>17</option>
                            <option value='18'>18</option>
                            <option value='19'>19</option>
                            <option value='20'>20</option>
                            <option value='21'>21</option>
                            <option value='22'>22</option>
                            <option value='23'>23</option>
                            <option value='24'>24</option>
                            <option value='25'>25</option>
                            <option value='26'>26</option>
                            <option value='27'>27</option>
                            <option value='28'>28</option>
                            <option value='29'>29</option>
                            <option value='30'>30</option>
                            <option value='31'>31</option>
                            <option value='32'>32</option>
                            <option value='33'>33</option>
                            <option value='34'>34</option>
                            <option value='35'>35</option>
                            <option value='36'>36</option>
                            <option value='37'>37</option>
                            <option value='38'>38</option>
                            <option value='39'>39</option>
                            <option value='40'>40</option>
                            <option value='41'>41</option>
                            <option value='42'>42</option>
                            <option value='43'>43</option>
                            <option value='44'>44</option>
                            <option value='45'>45</option>
                            <option value='46'>46</option>
                            <option value='47'>47</option>
                            <option value='48'>48</option>
                            <option value='49'>49</option>
                            <option value='50'>50</option>
                            <option value='51'>51</option>
                            <option value='52'>52</option>
                            <option value='53'>53</option>
                            <option value='54'>54</option>
                            <option value='55'>55</option>
                            <option value='56'>56</option>
                            <option value='57'>57</option>
                            <option value='58'>58</option>
                            <option value='59'>59</option>
                            <option value='60'>60</option>
                            <option value='61'>61</option>
                            <option value='62'>62</option>
                            <option value='63'>63</option>
                            <option value='64'>64</option>
                            <option value='65'>65</option>
                            <option value='66'>66</option>
                            <option value='67'>67</option>
                            <option value='68'>68</option>
                            <option value='69'>69</option>
                            <option value='70'>70</option>
                            <option value='71'>71</option>
                            <option value='72'>72</option>
                            <option value='73'>73</option>
                            <option value='74'>74</option>
                            <option value='75'>75</option>
                            <option value='76'>76</option>
                            <option value='77'>77</option>
                            <option value='78'>78</option>
                            <option value='79'>79</option>
                            <option value='80'>80</option>
                            <option value='81'>81</option>
                            <option value='82'>82</option>
                            <option value='83'>83</option>
                            <option value='84'>84</option>
                            <option value='85'>85</option>
                            <option value='86'>86</option>
                            <option value='87'>87</option>
                            <option value='88'>88</option>
                            <option value='89'>89</option>
                            <option value='90'>90</option>
                            <option value='91'>91</option>
                            <option value='92'>92</option>
                            <option value='93'>93</option>
                            <option value='94'>94</option>
                            <option value='95'>95</option>
                            <option value='96'>96</option>
                            <option value='97'>97</option>
                            <option value='98'>98</option>
                            <option value='99'>99</option>
                        </select>
                    </td>
                </tr>

                <tr>
                    <td class='labelTable'>RX_FREQ</td>
                    <td style='background-color: #FFFFFF;'> <input type='text' name='rx_freq' id='rx_freq' value='136.55000' size='9' maxlength='9'
                            aria-label='rx_freq' class='inputTable' onchange='inputRxFreq(this)'
                            onclick='getOldText(this.value)'>
                    </td>
                </tr>

                <tr>
                    <td class='labelTable'>CTS/DCS</td>
                    <td> <select name='cts_dcs' id='cts_dcs' class='selectTable' onchange='changeCTS_DCS(this.value)'>
                            <option value='0'>OFF</option>
                            <option value='1'>CTS</option>
                            <option value='2'>DCS</option>
                        </select>
                    </td>
                </tr>

                <tr>
                    <td class='labelTable'>CTS</td>
                    <td> <select name='rx_cts' id='rx_cts' disabled=true class='selectTable'
                            onchange='setParameter(this)'>
                            <option value='0'>OFF</option>
                            <option value='1'>67.0Hz</option>
                            <option value='2'>71.9Hz</option>
                            <option value='3'>74.4Hz</option>
                            <option value='4'>77.0Hz</option>
                            <option value='5'>79.7Hz</option>
                            <option value='6'>82.5Hz</option>
                            <option value='7'>85.4Hz</option>
                            <option value='8'>88.5Hz</option>
                            <option value='9'>91.5Hz</option>
                            <option value='10'>94.8Hz</option>
                            <option value='11'>97.4Hz</option>
                            <option value='12'>100.0Hz</option>
                            <option value='13'>103.5Hz</option>
                            <option value='14'>107.2Hz</option>
                            <option value='15'>110.9Hz</option>
                            <option value='16'>114.8Hz</option>
                            <option value='17'>118.8Hz</option>
                            <option value='18'>123.0Hz</option>
                            <option value='19'>127.3Hz</option>
                            <option value='20'>131.8Hz</option>
                            <option value='21'>136.5Hz</option>
                            <option value='22'>141.3Hz</option>
                            <option value='23'>146.2Hz</option>
                            <option value='24'>151.4Hz</option>
                            <option value='25'>156.7Hz</option>
                            <option value='26'>162.2Hz</option>
                            <option value='27'>167.9Hz</option>
                            <option value='28'>173.8Hz</option>
                            <option value='29'>179.9Hz</option>
                            <option value='30'>186.2Hz</option>
                            <option value='31'>192.8Hz</option>
                            <option value='32'>203.5Hz</option>
                            <option value='33'>210.7Hz</option>
                            <option value='34'>218.1Hz</option>
                            <option value='35'>225.7Hz</option>
                            <option value='36'>233.6Hz</option>
                            <option value='37'>241.8Hz</option>
                            <option value='38'>250.3Hz</option>

                        </select>
                    </td>
                </tr>

                <tr>
                    <td class='labelTable'>DCS</td>
                    <td> <select name='rx_dcs' id='rx_dcs' disabled=true class='selectTable'
                            onchange='setParameter(this)'>
                            <option value='0'>OFF</option>
                            <option value='1'>023N</option>
                            <option value='2'>025N</option>
                            <option value='3'>026N</option>
                            <option value='4'>031N</option>
                            <option value='5'>032N</option>
                            <option value='6'>043N</option>
                            <option value='7'>047N</option>
                            <option value='8'>051N</option>
                            <option value='9'>054N</option>
                            <option value='10'>065N</option>
                            <option value='11'>071N</option>
                            <option value='12'>072N</option>
                            <option value='13'>073N</option>
                            <option value='14'>074N</option>
                            <option value='15'>114N</option>
                            <option value='16'>115N</option>
                            <option value='17'>116N</option>
                            <option value='18'>125N</option>
                            <option value='19'>131N</option>
                            <option value='20'>132N</option>
                            <option value='21'>134N</option>
                            <option value='22'>143N</option>
                            <option value='23'>152N</option>
                            <option value='24'>155N</option>
                            <option value='25'>156N</option>
                            <option value='26'>162N</option>
                            <option value='27'>165N</option>
                            <option value='28'>172N</option>
                            <option value='29'>174N</option>
                            <option value='30'>205N</option>
                            <option value='31'>223N</option>
                            <option value='32'>226N</option>
                            <option value='33'>243N</option>
                            <option value='34'>244N</option>
                            <option value='35'>245N</option>
                            <option value='36'>251N</option>
                            <option value='37'>261N</option>
                            <option value='38'>263N</option>
                            <option value='39'>265N</option>
                            <option value='40'>271N</option>
                            <option value='41'>306N</option>
                            <option value='42'>311N</option>
                            <option value='43'>315N</option>
                            <option value='44'>331N</option>
                            <option value='45'>343N</option>
                            <option value='46'>346N</option>
                            <option value='47'>351N</option>
                            <option value='48'>364N</option>
                            <option value='49'>365N</option>
                            <option value='50'>371N</option>
                            <option value='51'>411N</option>
                            <option value='52'>412N</option>
                            <option value='53'>413N</option>
                            <option value='54'>423N</option>
                            <option value='55'>431N</option>
                            <option value='56'>432N</option>
                            <option value='57'>445N</option>
                            <option value='58'>464N</option>
                            <option value='59'>465N</option>
                            <option value='60'>466N</option>
                            <option value='61'>503N</option>
                            <option value='62'>506N</option>
                            <option value='63'>516N</option>
                            <option value='64'>532N</option>
                            <option value='65'>546N</option>
                            <option value='66'>565N</option>
                            <option value='67'>606N</option>
                            <option value='68'>612N</option>
                            <option value='69'>624N</option>
                            <option value='70'>627N</option>
                            <option value='71'>631N</option>
                            <option value='72'>632N</option>
                            <option value='73'>654N</option>
                            <option value='74'>662N</option>
                            <option value='75'>664N</option>
                            <option value='76'>703N</option>
                            <option value='77'>712N</option>
                            <option value='78'>723N</option>
                            <option value='79'>731N</option>
                            <option value='80'>732N</option>
                            <option value='81'>734N</option>
                            <option value='82'>743N</option>
                            <option value='83'>754N</option>

                        </select>
                    </td>
                </tr>

                <tr>
                    <td class='labelTable'>TX_FREQ</td>
                    <td style='background-color: #FFFFFF;'> <input type='text' name='tx_freq' id='tx_freq' value='136.55000' size='9' maxlength='9'
                            aria-label='tx_freq' class='inputTable' onchange='inputTxFreq(this)'
                            onclick='getOldText(this.value)'>
                    </td>
                </tr>

                <tr>
                    <td class='labelTable'>CTS</td>
                    <td> <select name='tx_cts' id='tx_cts' disabled=true class='selectTable'
                            onchange='setParameter(this)'>
                            <option value='0'>OFF</option>
                            <option value='1'>67.0Hz</option>
                            <option value='2'>71.9Hz</option>
                            <option value='3'>74.4Hz</option>
                            <option value='4'>77.0Hz</option>
                            <option value='5'>79.7Hz</option>
                            <option value='6'>82.5Hz</option>
                            <option value='7'>85.4Hz</option>
                            <option value='8'>88.5Hz</option>
                            <option value='9'>91.5Hz</option>
                            <option value='10'>94.8Hz</option>
                            <option value='11'>97.4Hz</option>
                            <option value='12'>100.0Hz</option>
                            <option value='13'>103.5Hz</option>
                            <option value='14'>107.2Hz</option>
                            <option value='15'>110.9Hz</option>
                            <option value='16'>114.8Hz</option>
                            <option value='17'>118.8Hz</option>
                            <option value='18'>123.0Hz</option>
                            <option value='19'>127.3Hz</option>
                            <option value='20'>131.8Hz</option>
                            <option value='21'>136.5Hz</option>
                            <option value='22'>141.3Hz</option>
                            <option value='23'>146.2Hz</option>
                            <option value='24'>151.4Hz</option>
                            <option value='25'>156.7Hz</option>
                            <option value='26'>162.2Hz</option>
                            <option value='27'>167.9Hz</option>
                            <option value='28'>173.8Hz</option>
                            <option value='29'>179.9Hz</option>
                            <option value='30'>186.2Hz</option>
                            <option value='31'>192.8Hz</option>
                            <option value='32'>203.5Hz</option>
                            <option value='33'>210.7Hz</option>
                            <option value='34'>218.1Hz</option>
                            <option value='35'>225.7Hz</option>
                            <option value='36'>233.6Hz</option>
                            <option value='37'>241.8Hz</option>
                            <option value='38'>250.3Hz</option>

                        </select>
                    </td>
                </tr>

                <tr>
                    <td class='labelTable'>DCS</td>
                    <td> <select name='tx_dcs' id='tx_dcs' disabled=true class='selectTable'
                            onchange='setParameter(this)'>
                            <option value='0'>OFF</option>
                            <option value='1'>023N</option>
                            <option value='2'>025N</option>
                            <option value='3'>026N</option>
                            <option value='4'>031N</option>
                            <option value='5'>032N</option>
                            <option value='6'>043N</option>
                            <option value='7'>047N</option>
                            <option value='8'>051N</option>
                            <option value='9'>054N</option>
                            <option value='10'>065N</option>
                            <option value='11'>071N</option>
                            <option value='12'>072N</option>
                            <option value='13'>073N</option>
                            <option value='14'>074N</option>
                            <option value='15'>114N</option>
                            <option value='16'>115N</option>
                            <option value='17'>116N</option>
                            <option value='18'>125N</option>
                            <option value='19'>131N</option>
                            <option value='20'>132N</option>
                            <option value='21'>134N</option>
                            <option value='22'>143N</option>
                            <option value='23'>152N</option>
                            <option value='24'>155N</option>
                            <option value='25'>156N</option>
                            <option value='26'>162N</option>
                            <option value='27'>165N</option>
                            <option value='28'>172N</option>
                            <option value='29'>174N</option>
                            <option value='30'>205N</option>
                            <option value='31'>223N</option>
                            <option value='32'>226N</option>
                            <option value='33'>243N</option>
                            <option value='34'>244N</option>
                            <option value='35'>245N</option>
                            <option value='36'>251N</option>
                            <option value='37'>261N</option>
                            <option value='38'>263N</option>
                            <option value='39'>265N</option>
                            <option value='40'>271N</option>
                            <option value='41'>306N</option>
                            <option value='42'>311N</option>
                            <option value='43'>315N</option>
                            <option value='44'>331N</option>
                            <option value='45'>343N</option>
                            <option value='46'>346N</option>
                            <option value='47'>351N</option>
                            <option value='48'>364N</option>
                            <option value='49'>365N</option>
                            <option value='50'>371N</option>
                            <option value='51'>411N</option>
                            <option value='52'>412N</option>
                            <option value='53'>413N</option>
                            <option value='54'>423N</option>
                            <option value='55'>431N</option>
                            <option value='56'>432N</option>
                            <option value='57'>445N</option>
                            <option value='58'>464N</option>
                            <option value='59'>465N</option>
                            <option value='60'>466N</option>
                            <option value='61'>503N</option>
                            <option value='62'>506N</option>
                            <option value='63'>516N</option>
                            <option value='64'>532N</option>
                            <option value='65'>546N</option>
                            <option value='66'>565N</option>
                            <option value='67'>606N</option>
                            <option value='68'>612N</option>
                            <option value='69'>624N</option>
                            <option value='70'>627N</option>
                            <option value='71'>631N</option>
                            <option value='72'>632N</option>
                            <option value='73'>654N</option>
                            <option value='74'>662N</option>
                            <option value='75'>664N</option>
                            <option value='76'>703N</option>
                            <option value='77'>712N</option>
                            <option value='78'>723N</option>
                            <option value='79'>731N</option>
                            <option value='80'>732N</option>
                            <option value='81'>734N</option>
                            <option value='82'>743N</option>
                            <option value='83'>754N</option>

                        </select>
                    </td>
                </tr>

                <tr>
                    <td class='labelTable'>POWER</td>
                    <td> <select name='power' id='power' class='selectTable' onchange='setParameter(this)'>
                            <option value='0'>HIGH</option>
                            <option value='1'>LOW</option>
                        </select>
                    </td>
                </tr>
                <tr>
                    <td class='labelTable'>BandWith</td>
                    <td> <select name='gbw' id='gbw' class='selectTable' onchange='setParameter(this)'>
                            <option value='0'>NARR</option>
                            <option value='1'>WIDE</option>
                        </select>
                    </td>
                </tr>

                <tr>
                    <td class='labelTable'>CHAN_NN</td>
                    <td style='background-color: #FFFFFF;'><input type='text' name='chan_nn' id='chan_nn' onchange='setParameter(this)'
                            aria-label='chan_nn' onclick='getOldText(this.value)' value='Hello' size='8' maxlength='7'
                            class='inputTable'>
                    </td>
                </tr>

                <tr>
                    <td class='labelTable'>NOW MODE</td>
                    <td> <select name='nowMode' id='nowMode' class='selectTable' onchange='setParameter(this)'>
                            <option value='0'>CHANNEL</option>
                            <option value='1'>VHF</option>
                            <option value='2'>UHF</option>
                        </select>
                    </td>
                </tr>

                <tr>
                    <td class='titleTableBlank' colspan='2' align='center' height='30'> </td>
                </tr>

            </table>
        </div>

        <div style='display:inline-block;'>
            <table border='1' cellpadding='0' class='table'>
                <tr>
                    <td class='titleTable' colspan='2' align='center' height='30'>PGM Setting</td>
                </tr>

                <tr>
                    <td class='labelTable'>PROGRAM</td>
                    <td class='labelTable'>VALUE</td>
                </tr>

                <tr>
                    <td class='labelTable'>AUDIO_SET</td>
                    <td> <select name='audioSet' id='audioSet' class='selectTable' onchange='setParameter(this)'>
                            <option value='0'>IN</option>
                            <option value='1'>TOP_LOW</option>
                            <option value='2'>TOP_MID</option>
                            <option value='3'>TOP_HIGH</option>
                            <option value='4'>SIDE_LOW</option>
                            <option value='5'>SIDE_MID</option>
                            <option value='6'>SIDE_HIGH</option>
                        </select>
                    </td>
                </tr>
                <tr>
                    <td class='labelTable'>SQL</td>
                    <td> <select name='sql' id='sql' class='selectTable' onchange='setParameter(this)'>
                            <option value='0'>0</option>
                            <option value='1'>1</option>
                            <option value='2'>2</option>
                            <option value='3'>3</option>
                            <option value='4'>4</option>
                            <option value='5'>5</option>
                            <option value='6'>6</option>
                            <option value='7'>7</option>
                            <option value='8'>8</option>
                        </select>
                    </td>
                </tr>
                <tr>
                    <td class='labelTable'>STEP</td>
                    <td> <select name='step' id='step' class='selectTable' onchange='setParameter(this)'>
                            <option value='0'>5K</option>
                            <option value='1'>10K</option>
                            <option value='2'>12.5K</option>
                        </select>
                    </td>
                </tr>
                <tr>
                    <td class='labelTable'>TOT</td>
                    <td> <select name='tot' id='tot' class='selectTable' onchange='setParameter(this)'>
                            <option value='0'>OFF</option>
                            <option value='1'>1</option>
                            <option value='2'>2</option>
                            <option value='3'>3</option>
                            <option value='4'>4</option>
                            <option value='5'>5</option>
                            <option value='6'>6</option>
                            <option value='7'>7</option>
                            <option value='8'>8</option>
                            <option value='9'>9</option>
                        </select>
                    </td>
                </tr>

                <tr>
                    <td class='labelTable'>LampTime</td>
                    <td> <select name='lampTime' id='lampTime' class='selectTable' onchange='setParameter(this)'>
                            <option value='0'>ALWAYS</option>
                            <option value='1'>AUTO</option>
                        </select>
                    </td>
                </tr>

                <tr>
                    <td class='labelTable'>VDC-OP</td>
                    <td> <select name='topPowerOut' id='topPowerOut' class='selectTable' onchange='setParameter(this)'>
                            <option value='0'>OFF</option>
                            <option value='1'>ON</option>
                        </select>
                    </td>
                </tr>

                <tr>
                    <td class='labelTable'>PTT_TONE</td>
                    <td> <select name='pttTone' id='pttTone' class='selectTable' onchange='setParameter(this)'>
                            <option value='0'>PRE+END</option>
                            <option value='1'>PRE</option>
                            <option value='2'>END</option>
                            <option value='3'>OFF</option>
                        </select>
                    </td>
                </tr>
                <tr>
                    <td class='titleTableNoRaidus' colspan='2' align='center' height='30'>DUAL MODE</td>
                </tr>
                <tr>
                    <td class='labelTable'>CHAN A</td>
                    <td> <select name='chanA' id='chanA' class='selectTable' onchange='setParameter(this)'>
                            <option value='1'>1</option>
                            <option value='2'>2</option>
                            <option value='3'>3</option>
                            <option value='4'>4</option>
                            <option value='5'>5</option>
                            <option value='6'>6</option>
                            <option value='7'>7</option>
                            <option value='8'>8</option>
                            <option value='9'>9</option>
                            <option value='10'>10</option>
                            <option value='11'>11</option>
                            <option value='12'>12</option>
                            <option value='13'>13</option>
                            <option value='14'>14</option>
                            <option value='15'>15</option>
                            <option value='16'>16</option>
                            <option value='17'>17</option>
                            <option value='18'>18</option>
                            <option value='19'>19</option>
                            <option value='20'>20</option>
                            <option value='21'>21</option>
                            <option value='22'>22</option>
                            <option value='23'>23</option>
                            <option value='24'>24</option>
                            <option value='25'>25</option>
                            <option value='26'>26</option>
                            <option value='27'>27</option>
                            <option value='28'>28</option>
                            <option value='29'>29</option>
                            <option value='30'>30</option>
                            <option value='31'>31</option>
                            <option value='32'>32</option>
                            <option value='33'>33</option>
                            <option value='34'>34</option>
                            <option value='35'>35</option>
                            <option value='36'>36</option>
                            <option value='37'>37</option>
                            <option value='38'>38</option>
                            <option value='39'>39</option>
                            <option value='40'>40</option>
                            <option value='41'>41</option>
                            <option value='42'>42</option>
                            <option value='43'>43</option>
                            <option value='44'>44</option>
                            <option value='45'>45</option>
                            <option value='46'>46</option>
                            <option value='47'>47</option>
                            <option value='48'>48</option>
                            <option value='49'>49</option>
                            <option value='50'>50</option>
                            <option value='51'>51</option>
                            <option value='52'>52</option>
                            <option value='53'>53</option>
                            <option value='54'>54</option>
                            <option value='55'>55</option>
                            <option value='56'>56</option>
                            <option value='57'>57</option>
                            <option value='58'>58</option>
                            <option value='59'>59</option>
                            <option value='60'>60</option>
                            <option value='61'>61</option>
                            <option value='62'>62</option>
                            <option value='63'>63</option>
                            <option value='64'>64</option>
                            <option value='65'>65</option>
                            <option value='66'>66</option>
                            <option value='67'>67</option>
                            <option value='68'>68</option>
                            <option value='69'>69</option>
                            <option value='70'>70</option>
                            <option value='71'>71</option>
                            <option value='72'>72</option>
                            <option value='73'>73</option>
                            <option value='74'>74</option>
                            <option value='75'>75</option>
                            <option value='76'>76</option>
                            <option value='77'>77</option>
                            <option value='78'>78</option>
                            <option value='79'>79</option>
                            <option value='80'>80</option>
                            <option value='81'>81</option>
                            <option value='82'>82</option>
                            <option value='83'>83</option>
                            <option value='84'>84</option>
                            <option value='85'>85</option>
                            <option value='86'>86</option>
                            <option value='87'>87</option>
                            <option value='88'>88</option>
                            <option value='89'>89</option>
                            <option value='90'>90</option>
                            <option value='91'>91</option>
                            <option value='92'>92</option>
                            <option value='93'>93</option>
                            <option value='94'>94</option>
                            <option value='95'>95</option>
                            <option value='96'>96</option>
                            <option value='97'>97</option>
                            <option value='98'>98</option>
                            <option value='99'>99</option>
                        </select>
                    </td>
                </tr>
                <tr>
                    <td class='labelTable'>CHAN B</td>
                    <td> <select name='chanB' id='chanB' class='selectTable' onchange='setParameter(this)'>
                            <option value='1'>1</option>
                            <option value='2'>2</option>
                            <option value='3'>3</option>
                            <option value='4'>4</option>
                            <option value='5'>5</option>
                            <option value='6'>6</option>
                            <option value='7'>7</option>
                            <option value='8'>8</option>
                            <option value='9'>9</option>
                            <option value='10'>10</option>
                            <option value='11'>11</option>
                            <option value='12'>12</option>
                            <option value='13'>13</option>
                            <option value='14'>14</option>
                            <option value='15'>15</option>
                            <option value='16'>16</option>
                            <option value='17'>17</option>
                            <option value='18'>18</option>
                            <option value='19'>19</option>
                            <option value='20'>20</option>
                            <option value='21'>21</option>
                            <option value='22'>22</option>
                            <option value='23'>23</option>
                            <option value='24'>24</option>
                            <option value='25'>25</option>
                            <option value='26'>26</option>
                            <option value='27'>27</option>
                            <option value='28'>28</option>
                            <option value='29'>29</option>
                            <option value='30'>30</option>
                            <option value='31'>31</option>
                            <option value='32'>32</option>
                            <option value='33'>33</option>
                            <option value='34'>34</option>
                            <option value='35'>35</option>
                            <option value='36'>36</option>
                            <option value='37'>37</option>
                            <option value='38'>38</option>
                            <option value='39'>39</option>
                            <option value='40'>40</option>
                            <option value='41'>41</option>
                            <option value='42'>42</option>
                            <option value='43'>43</option>
                            <option value='44'>44</option>
                            <option value='45'>45</option>
                            <option value='46'>46</option>
                            <option value='47'>47</option>
                            <option value='48'>48</option>
                            <option value='49'>49</option>
                            <option value='50'>50</option>
                            <option value='51'>51</option>
                            <option value='52'>52</option>
                            <option value='53'>53</option>
                            <option value='54'>54</option>
                            <option value='55'>55</option>
                            <option value='56'>56</option>
                            <option value='57'>57</option>
                            <option value='58'>58</option>
                            <option value='59'>59</option>
                            <option value='60'>60</option>
                            <option value='61'>61</option>
                            <option value='62'>62</option>
                            <option value='63'>63</option>
                            <option value='64'>64</option>
                            <option value='65'>65</option>
                            <option value='66'>66</option>
                            <option value='67'>67</option>
                            <option value='68'>68</option>
                            <option value='69'>69</option>
                            <option value='70'>70</option>
                            <option value='71'>71</option>
                            <option value='72'>72</option>
                            <option value='73'>73</option>
                            <option value='74'>74</option>
                            <option value='75'>75</option>
                            <option value='76'>76</option>
                            <option value='77'>77</option>
                            <option value='78'>78</option>
                            <option value='79'>79</option>
                            <option value='80'>80</option>
                            <option value='81'>81</option>
                            <option value='82'>82</option>
                            <option value='83'>83</option>
                            <option value='84'>84</option>
                            <option value='85'>85</option>
                            <option value='86'>86</option>
                            <option value='87'>87</option>
                            <option value='88'>88</option>
                            <option value='89'>89</option>
                            <option value='90'>90</option>
                            <option value='91'>91</option>
                            <option value='92'>92</option>
                            <option value='93'>93</option>
                            <option value='94'>94</option>
                            <option value='95'>95</option>
                            <option value='96'>96</option>
                            <option value='97'>97</option>
                            <option value='98'>98</option>
                            <option value='99'>99</option>
                        </select>
                    </td>
                </tr>

                <tr>
                    <td class='titleTableNoRaidus' colspan='2' align='center' height='35' style='color: black;'>
                        Per 1.0.000 </td>
                </tr>
                <tr>
                    <td class='titleTableBlank' colspan='2' align='center' height='35'>
                        FCS AN/PRC 152(A) </td>
                </tr>

            </table>

            <hr size='2px' noshade='true'>

            <form action='/finish' method='post'>
                <input type='submit' value='Finish' style='width: 150px; height: 25px; font-weight: 500; font-size: 18px;'>
            </form>

        </div>

    </div>


</body>

<script>
    var oldFreq = 435.25000;
    function getOldText(val) {
        // alert('原来值:' + val)
        oldFreq = val;
    }
    //返回值0: 合法; 非0: 校正频率
    function checkFreq(val) {
        var step = 0;
        var _step = [500, 625];
        var freq = Number(val) * 100000;

        for (var i = 0; i < 2; i++) {   //若是能整除,则返回合法
            if (freq % _step[i] == 0)
                return 0;
        }
        //不合法,以默认步进校正
        var mul = Math.floor(freq / _step[step]);   //0.3为四舍五入校正参数
        // alert('矫正倍数:' + mul);
        // alert('矫正:' + mul * _step[step]);
        // alert('矫正后数据:' + (mul * _step[step] / 10000));
        return (mul * _step[step] / 100000);
    }
    //格式化需要输出的频率
    function formatFreq(val) {
        var freq = String(val);
        // alert('freq长度:' + freq.length + ',val类型:' + typeof (val) + ',freq类型:' + typeof (freq))
        if (freq.length < 9) {
            if (freq.length < 4) {
                freq = freq + '.' + '0'.repeat(9 - freq.length - 1);
            }
            else {
                freq = freq + '0'.repeat(9 - freq.length);
            }
        }
        return freq;
    }
    //校验修改的Rx频率
    function inputRxFreq(module) {
        var freq = module.value;
        var result = 0;
        if (freq == oldFreq) {
            document.getElementById('rx_freq').value = oldFreq;
            return;
        }
        // alert('input rx_freq:' + document.getElementById('rx_freq').value + ' val:' + val);
        var chan = document.getElementById('chan').value;
        if (chan > 0 && chan < 100) {
            // alert('当前chan:' + chan);
            if (freq < 136.0 || (freq > 174.0 && freq < 400.0) || freq > 480.0) {
                document.getElementById('rx_freq').value = oldFreq;
                return;
            }

        } else if (chan == 0) {
            if (freq > 174.0 || freq < 136.0) {
                alert('VHF 输入错误');
                document.getElementById('rx_freq').value = oldFreq;
                return;
            }
        } else if (chan == 100) {
            if (freq > 480.0 || freq < 400.0) {
                alert('UHF 输入错误');
                document.getElementById('rx_freq').value = oldFreq;
                return;
            }
        }
        result = checkFreq(freq);
        // alert('rxResult:' + result);
        if (result == 0) {
            freq = formatFreq(freq);
            document.getElementById('rx_freq').value = freq;
            document.getElementById('tx_freq').value = freq;
            setParameter(module);
            setParameter(document.getElementById('tx_freq'));
            return;
        };
        result = formatFreq(result);
        document.getElementById('rx_freq').value = result;
        document.getElementById('tx_freq').value = result;
        setParameter(module);
        setParameter(document.getElementById('tx_freq'));
    }
    //校验修改的Tx频率
    function inputTxFreq(module) {
        var freq = module.value
        var result = 0;
        if (freq == oldFreq) {
            document.getElementById('tx_freq').value = oldFreq;
            return;
        }
        // alert('input tx_freq:' + document.getElementById('tx_freq').value + ' val:' + val);
        var chan = document.getElementById('chan').value;
        if (chan > 0 && chan < 100) {
            // alert('当前chan:' + chan);
            if (freq < 136.0 || (freq > 174.0 && freq < 400.0) || freq > 480.0) {
                document.getElementById('tx_freq').value = oldFreq;
                return;
            }

        } else if (chan == 0) {
            if (freq > 174.0 || freq < 136.0) {
                alert('VHF 输入错误');
                document.getElementById('tx_freq').value = oldFreq;
                return;
            }
        } else if (chan == 100) {
            if (freq > 480.0 || freq < 400.0) {
                alert('UHF 输入错误');
                document.getElementById('tx_freq').value = oldFreq;
                return;
            }
        }
        result = checkFreq(freq);
        // alert('txResult:' + result);
        if (result == 0) {
            freq = formatFreq(freq);
            document.getElementById('tx_freq').value = freq;
            setParameter(module);
            return;
        }
        result = formatFreq(result);
        document.getElementById('tx_freq').value = result;
        setParameter(module);
    }

    function inputChanNN(module) {
        if (module.value == oldFreq)
            return;
        setParameter(module);
    }

    function CalAud(aud, mic) {
        var ret = 0;
        if (aud == 0) ret = 0;
        else if (aud == 1) {
            ret = mic + 1;
        }
        else if (aud == 2) {
            ret = mic + 4;
        }
        return ret;
    }

    function handleDisplayCTSDCS(rs, ts) {
        if (rs == 0 && ts == 0) {
            changeCTS_DCS('0');
        } else if (rs > 38 || ts > 38) {
            changeCTS_DCS('2');
            if (rs > 38) {
                document.getElementById('rx_dcs').value = rs - 38 + 1;
            }
            if (ts > 38) {
                document.getElementById('tx_dcs').value = ts - 38 + 1;
            }
        } else {
            changeCTS_DCS('1');
            document.getElementById('rx_cts').value = rs;
            document.getElementById('tx_cts').value = ts;
        }
    }

    function changeCTS_DCS(val) {
        // alert('cts_dcs:' + val + typeof (val));
        document.getElementById('cts_dcs').value = val;
        switch (Number(val)) {
            case 0:
                document.getElementById('rx_cts').disabled = true;
                document.getElementById('rx_dcs').disabled = true;
                document.getElementById('tx_cts').disabled = true;
                document.getElementById('tx_dcs').disabled = true;
                document.getElementById('rx_cts').value = 0;
                document.getElementById('rx_dcs').value = 0;
                document.getElementById('tx_cts').value = 0;
                document.getElementById('tx_dcs').value = 0;

                break;
            case 1:
                document.getElementById('rx_cts').disabled = false;
                document.getElementById('rx_dcs').disabled = true;
                document.getElementById('rx_dcs').value = 0;
                document.getElementById('tx_cts').disabled = false;
                document.getElementById('tx_dcs').disabled = true;
                document.getElementById('tx_dcs').value = 0;
                break;
            case 2:
                document.getElementById('rx_cts').disabled = true;
                document.getElementById('rx_cts').value = 0;
                document.getElementById('rx_dcs').disabled = false;
                document.getElementById('tx_cts').disabled = true;
                document.getElementById('tx_cts').value = 0;
                document.getElementById('tx_dcs').disabled = false;
                break;

        }

    }

    function getChanParameter(module) {
        var chan = module.value;
        // alert('当前chan:' + chan)
        xmlhttp = new XMLHttpRequest();
        xmlhttp.onreadystatechange = function () {
            if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
                // alert('fun xmlhttp:' + xmlhttp.responseText);
                var str = xmlhttp.responseText;
                var jsonReturn = window.JSON.parse(str);
                var rs = jsonReturn.rs;
                var ts = jsonReturn.ts;
                // alert('chan'+jsonReturn.chan);
                document.getElementById('chan').value = jsonReturn.chan;
                document.getElementById('rx_freq').value = formatFreq(jsonReturn.rx_freq);
                document.getElementById('tx_freq').value = formatFreq(jsonReturn.tx_freq);
                document.getElementById('gbw').value = jsonReturn.gbw;
                document.getElementById('power').value = jsonReturn.power;
                document.getElementById('chan_nn').value = jsonReturn.chan_nn;
                handleDisplayCTSDCS(rs, ts);
                if (jsonReturn.chan == 0 || jsonReturn.chan == 100)
                    document.getElementById('chan_nn').disabled = true;
                else
                    document.getElementById('chan_nn').disabled = false;

            }
        }
        xmlhttp.open('GET', '/getChan?chan=' + chan, true);
        xmlhttp.send();
    }

    function getAll() {
        xmlhttp = new XMLHttpRequest();
        xmlhttp.onreadystatechange = function () {
            if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
                var str = xmlhttp.responseText;
                var jsonReturn = window.JSON.parse(str);
                // alert('chan'+jsonReturn.chan);
                document.getElementById('chan').value = jsonReturn.chan;
                document.getElementById('rx_freq').value = formatFreq(jsonReturn.rx_freq);
                document.getElementById('tx_freq').value = formatFreq(jsonReturn.tx_freq);
                document.getElementById('gbw').value = jsonReturn.gbw;
                document.getElementById('power').value = jsonReturn.power;
                document.getElementById('chan_nn').value = jsonReturn.chan_nn;
                document.getElementById('chanA').value = jsonReturn.chanA;
                document.getElementById('chanB').value = jsonReturn.chanB;

                var rs = jsonReturn.rs;
                var ts = jsonReturn.ts;
                var cf = jsonReturn.cf;
                var vu = jsonReturn.vu;
                var aud = jsonReturn.aud;
                var mic = jsonReturn.mic;
                var sql = jsonReturn.sql;
                var step = jsonReturn.step;
                var tot = jsonReturn.tot;
                var lampTime = jsonReturn.lampTime;
                var topPowerOut = jsonReturn.topPowerOut;
                var tone = jsonReturn.preTone * 2 + jsonReturn.endTone;

                handleDisplayCTSDCS(rs, ts);

                document.getElementById('nowMode').value = cf * (vu + 1);

                document.getElementById('audioSet').value = CalAud(aud, mic);
                document.getElementById('sql').value = sql;
                document.getElementById('step').value = step;
                document.getElementById('tot').value = tot;
                document.getElementById('lampTime').value = lampTime;
                document.getElementById('topPowerOut').value = topPowerOut;
                document.getElementById('pttTone').value = 3 - tone;
            }
        }
        xmlhttp.open('GET', '/getAll', true);
        xmlhttp.send();
    }


    function setParameter(module) {
        // alert('修改' + module.id);
        if ((module.id == 'chan_nn' && module.value == oldFreq) ||
            (Number(module.value) == Number(oldFreq)))
            return;

        var par = 0;
        xmlhttp = new XMLHttpRequest();
        xmlhttp.onreadystatechange = function () {
            if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
                var str = xmlhttp.responseText;
                // alert('return:' + str + ',' + (str == 'OK'));
                if (str != 'OK')
                    alert('Error: ' + str);
            }
        }

        // alert('修改:' + module.id + ', value:' + module.value);
        if (module.id == 'rx_cts')
            par = 'rs' + '=' + module.value;
        else if (module.id == 'tx_cts')
            par = 'ts' + '=' + module.value;
        else if (module.id == 'rx_dcs')
            par = 'rs' + '=' + (module.value + 38);
        else if (module.id == 'tx_dcs')
            par = 'ts' + '=' + (module.value + 38);
        else
            par = module.id + '=' + module.value;

        // alert('拼接结果:' + par);
        xmlhttp.open('POST', '/set?' + par, true);
        xmlhttp.send();
    }

</script>

</html>
)";