console.log("hello");

const Parameters = [
    "Temp. 1",
    "Temp. 2",
    "Temp. Δ",
    "Temp. 1 Status",
    "Temp. 2 Status",
    "Temp. 1+2 Status",
    "Temp. Internal",
    "Tick time"
]

var timeRequest = {
    "type": "time",
    "id": 1,
    "options": {
      "instance": "fdirdemo"
    }
  }

var parameterRequest = {
    "type": "parameters",
    "id": 2,
    "options": {
        "instance": "fdirdemo",
        "processor": "realtime",
        "id": [
            { name: "/fdirdemo/PMON_Monitoring_Definition"},
            { name: "/fdirdemo/PMON_Limit_Check_Double"},
            { name: "/fdirdemo/PMON_Limit_Check_Float"},
            { name: "/fdirdemo/PMON_Expected_Value_Check_uint64"},
            { name: "/fdirdemo/PMON_Expected_Value_Check_Temperature_Status"},
            { name: "/fdirdemo/EventAction_List" },
            { name: "/fdirdemo/EventAction_Definition" },
            { name: "/fdirdemo/PMON_Check_Transition_List" },
        ]
    }
}

var EventEnumeration = {}
var CheckStatusEnumeration = {}

var websocket = new WebSocket("ws://localhost:8090/api/websocket")

websocket.onopen = function (event) {
    setTimeout(function() {
        websocket.send(JSON.stringify(timeRequest));
        websocket.send(JSON.stringify(parameterRequest));
    }, 300);
}

const $timestamp = document.getElementById('timestamp');
const $pmonTable = document.getElementById('pmon-table');
const $eventActionTable = document.getElementById('event-action-table');
const $transitionTable = document.getElementById('check-transition-table');

let pmons = {}
let events = {};
let transitions = [];

findkey = function(array) {
    return function(key) {
        var index = _.findIndex(array.name, function(e) { return e == key })
        return array.value[index];
    }
}

getEnumerations = function() {
    var http = new XMLHttpRequest();
    http.addEventListener("load", function() {
        var json = JSON.parse(this.responseText);
        for (var value of json.type.enumValue) {
            EventEnumeration[parseInt(value.value)] = value.label;
        }
    })
    http.open("GET", "http://localhost:8090/api/mdb/fdirdemo/parameters/fdirdemo/Event_Definition_ID");
    http.send();

    var http2 = new XMLHttpRequest();
    http2.addEventListener("load", function() {
        var json = JSON.parse(this.responseText);
        for (var value of _.find(json.type.member, {name: 'Check_Status'}).type.enumValue) {
            CheckStatusEnumeration[parseInt(value.value)] = value.label;
        }
    })
    http2.open("GET", "http://localhost:8090/api/mdb/fdirdemo/parameters/fdirdemo/PMON_Monitoring_Definition");
    http2.send();
}
getEnumerations();

getST12definitions = _.throttle(function() {
    pmons = {};
    var http = new XMLHttpRequest();
    http.open("POST",
        "http://localhost:8090/api/processors/fdirdemo/realtime/commands/fdirdemo/ST12_ListAllDefinitions");
    http.send();
}, 200, { 'leading': false, 'trailing': true });

getST19definitions = function() {
    events = {};
    var http = new XMLHttpRequest();
    http.open("POST",
        "http://localhost:8090/api/processors/fdirdemo/realtime/commands/fdirdemo/ST19_ListAllEventAction");
    http.send();
}

getST19requests = function() {
    for (key in events) {
        console.log(key)
        var http = new XMLHttpRequest();
        http.open("POST",
            "http://localhost:8090/api/processors/fdirdemo/realtime/commands/fdirdemo/ST19_ListEventActionRequest");
        http.send(JSON.stringify({
            assignment: [
                {name: "Event_Action_Definition_ID", value: key }
            ]
        }));
    }
}

clearTransitions = function() {
    transitions = [];
    createTransitionTable();
}

colourStatus = function(element)  {
    text = element.innerText.trim();
    if (text == "Invalid") {
        element.style.color = "#ff8f00";
        element.style.fontWeight = 600;
    } else if (text == "Unchecked") {
        element.style.colour = "#757575";
        element.style.fontWeight = 500;
    } else if (text != "OK") {
        element.style.color = "#c62828";
        element.style.fontWeight = 600;
    }
    return element;
}

createPmonTable = _.throttle(function() {
    $pmonTable.innerHTML = '';

    for (const [pmonId, pmon] of Object.entries(pmons)) {
        var tr = document.createElement('tr');

        var tds = _.map(new Array(8), function (e) { return document.createElement('td')});

        tds[0].appendChild(document.createTextNode(pmonId));
        tds[1].appendChild(document.createTextNode(pmon.parameter));
        tds[1].classList.add("table-parameter");

        if (pmon.validity) {
            var lis = _.map(new Array(2), function (e) { return document.createElement('p')});
            lis[0].appendChild(document.createTextNode(pmon.validity.parameter));
            lis[1].appendChild(document.createTextNode(pmon.validity.value));
            lis[1].appendChild(document.createElement('span'));
            lis[1].childNodes[1].classList.add('mdl-chip');
            lis[1].childNodes[1].classList.add('mdl-chip-table');
            lis[1].childNodes[1].appendChild(document.createElement('code'));
            lis[1].childNodes[1].childNodes[0].classList.add('mdl-chip__text');
            lis[1].childNodes[1].childNodes[0].appendChild(document.createTextNode(pmon.validity.mask))
            tds[2].appendChild(lis[0]);
            tds[2].appendChild(lis[1]);
        }

        tds[3].appendChild(document.createTextNode(pmon.monitoring_interval));
        tds[4].appendChild(document.createTextNode(pmon.status));

        colourStatus(tds[4]);

        tds[5].appendChild(document.createTextNode(pmon.repetition_number));

        if (pmon.check_type == "Limit_Check") {
            nodes = []
            nodes[0] = document.createElement('p');
            nodes[0].appendChild(document.createTextNode(pmon.check.low + " <= " + "x" + " <= " + pmon.check.high));
            nodes[1] = document.createElement('span');
            nodes[1].classList.add('mdl-chip');
            nodes[1].classList.add('mdl-chip-long');
            nodes[1].appendChild(document.createElement('span'));
            nodes[1].childNodes[0].classList.add('mdl-chip__text');
            nodes[1].childNodes[0].appendChild(document.createTextNode(pmon.check.low_event))
            nodes[2] = document.createElement('span');
            nodes[2].classList.add('mdl-chip');
            nodes[2].classList.add('mdl-chip-long');
            nodes[2].appendChild(document.createElement('span'));
            nodes[2].childNodes[0].classList.add('mdl-chip__text');
            nodes[2].childNodes[0].appendChild(document.createTextNode(pmon.check.high_event))

            tds[6].appendChild(nodes[0])
            tds[6].appendChild(nodes[1])
            tds[6].appendChild(nodes[2])
        } else {
            nodes = []
            nodes[0] = document.createElement('p');
            nodes[0].appendChild(document.createTextNode("x" + " = " + pmon.check.value))
            nodes[1] = document.createElement('span');
            nodes[1].classList.add('mdl-chip');
            nodes[1].classList.add('mdl-chip-table');
            nodes[1].appendChild(document.createElement('span'));
            nodes[1].childNodes[0].classList.add('mdl-chip__text');
            nodes[1].childNodes[0].appendChild(document.createTextNode(pmon.check.mask))
            nodes[0].appendChild(nodes[1]);
            nodes[2] = document.createElement('span');
            nodes[2].classList.add('mdl-chip');
            nodes[2].classList.add('mdl-chip-long');
            nodes[2].appendChild(document.createElement('span'));
            nodes[2].childNodes[0].classList.add('mdl-chip__text');
            nodes[2].childNodes[0].appendChild(document.createTextNode(pmon.check.event))

            tds[6].appendChild(nodes[0])
            tds[6].appendChild(nodes[2])
        }

        tds[7].appendChild(document.createTextNode(pmon.date))

        for (const td of Object.values(tds)) {
            tr.appendChild(td);
        }

        $pmonTable.appendChild(tr);
    }
}, 50, {'leading': false, 'trailing': true});

createEventActionTable = _.throttle(function() {
    $eventActionTable.innerHTML = '';

    for (const [eventActionID, eventAction] of Object.entries(events)) {
        var tr = document.createElement('tr');
        var tds = _.map(new Array(4), function (e) { return document.createElement('td')});

        tds[0].appendChild(document.createTextNode(eventActionID));

        tds[1].appendChild(document.createElement('span'));
        tds[1].childNodes[0].classList.add('mdl-chip');
        // tds[1].childNodes[0].classList.add('mdl-chip-long');
        tds[1].childNodes[0].appendChild(document.createElement('span'));
        tds[1].childNodes[0].childNodes[0].classList.add('mdl-chip__text');
        tds[1].childNodes[0].childNodes[0].appendChild(document.createTextNode(eventAction.event))

        if (eventAction.enabled) {
            tds[2].appendChild(document.createTextNode("On"));
            tds[2].classList.add('mdl-color-text--teal-300');
        } else {
            tds[2].appendChild(document.createTextNode("Off"));
            tds[2].classList.add('mdl-color-text--red-300');
        }

        if (eventAction.request) {
            tds[3].appendChild(document.createTextNode("TC[" 
                + eventAction.request.service + "," + eventAction.request.message + "] "));
            tds[3].appendChild(document.createElement('span'));
            tds[3].childNodes[1].classList.add('mdl-typography--font-light');
            tds[3].childNodes[1].appendChild(document.createTextNode(eventAction.request.content1 + " "));
            tds[3].appendChild(document.createElement('span'));
            tds[3].childNodes[2].classList.add('mdl-typography--font-light');
            tds[3].childNodes[2].appendChild(document.createTextNode(eventAction.request.content2));
        }

        for (const td of Object.values(tds)) {
            tr.appendChild(td);
        }
        $eventActionTable.appendChild(tr);
    }
}, 50, {'leading': false, 'trailing': true});

createTransitionTable = _.throttle(function() {
    $transitionTable.innerHTML = '';

    for (const transition of _.reverse([...transitions])) {
        var tr = document.createElement('tr');
        var tds = _.map(new Array(5), function (e) { return document.createElement('td')});

        tds[0].appendChild(document.createTextNode(transition.timestamp));
        tds[1].appendChild(document.createTextNode(transition.pmonId));
        tds[2].appendChild(document.createTextNode(transition.parameter));
        tds[3].appendChild(document.createTextNode(transition.value));

        tds[4].appendChild(document.createElement('span'));
        tds[4].appendChild(document.createElement('i'));
        tds[4].appendChild(document.createElement('span'));
        tds[4].childNodes[0].appendChild(document.createTextNode(transition.previous));
        colourStatus(tds[4].childNodes[0]);
        tds[4].childNodes[1].appendChild(document.createTextNode('arrow_right_alt'));
        tds[4].childNodes[1].classList.add('material-icons');
        tds[4].childNodes[2].appendChild(document.createTextNode(transition.current));
        colourStatus(tds[4].childNodes[2]);


        for (const td of Object.values(tds)) {
            tr.appendChild(td);
        }
        $transitionTable.appendChild(tr);
    }
}, 50, {'leading': false, 'trailing': true});

websocket.onmessage = function (event) {
    var json = JSON.parse(event.data);
    console.log(json);

    if (json.type == "time") {
        $timestamp.innerText = json.data.value;
    } else if (json.type == "parameters") {
        var monitoringRaw = _.find(json.data.values, {'numericId': 1});
        var eventActionRaw = _.find(json.data.values, {'numericId': 6});
        var eventActionRequestRaw = _.find(json.data.values, {'numericId': 7});
        var checkListRaw = _.find(json.data.values, {'numericId': 8});

        if (eventActionRaw) {
            var eventActionList = eventActionRaw.engValue.arrayValue;

            for (var definitionRaw of eventActionList) {
                var eventaction = findkey(definitionRaw.aggregateValue);

                var entry = {
                    "event": EventEnumeration[eventaction("Event_Definition_ID").uint32Value],
                    "ID": eventaction("EventAction_Definition_ID").uint32Value,
                    "enabled": eventaction("Enabled").uint32Value
                }

                events[entry["ID"]] = entry;
            }

            createEventActionTable();
        }
        if (eventActionRequestRaw) {
            var eventActionRequest = findkey(eventActionRequestRaw.engValue.aggregateValue);
            var id = eventActionRequest("EventAction_Definition_ID").uint32Value;
            var text = eventActionRequest("TC").stringValue;
            var request = {
                text: text,
                service: text.charCodeAt(1),
                message: text.charCodeAt(2),
                content: text.substr(5),
                content1: text.substr(5, 16),
                content2: text.substr(5 + 16, 16)
            }
createEventActionTable = _.throttle(function() {
    $eventActionTable.innerHTML = '';

    for (const [eventActionID, eventAction] of Object.entries(events)) {
        var tr = document.createElement('tr');
        var tds = _.map(new Array(4), function (e) { return document.createElement('td')});

        tds[0].appendChild(document.createTextNode(eventActionID));

        tds[1].appendChild(document.createElement('span'));
        tds[1].childNodes[0].classList.add('mdl-chip');
        // tds[1].childNodes[0].classList.add('mdl-chip-long');
        tds[1].childNodes[0].appendChild(document.createElement('span'));
        tds[1].childNodes[0].childNodes[0].classList.add('mdl-chip__text');
        tds[1].childNodes[0].childNodes[0].appendChild(document.createTextNode(eventAction.event))

        if (eventAction.enabled) {
            tds[2].appendChild(document.createTextNode("On"));
            tds[2].classList.add('mdl-color-text--teal-300');
        } else {
            tds[2].appendChild(document.createTextNode("Off"));
            tds[2].classList.add('mdl-color-text--red-300');
        }

        if (eventAction.request) {
            tds[3].appendChild(document.createTextNode("TC[" + 
                eventAction.request.service + "," + eventAction.request.message + "] "));
            tds[3].appendChild(document.createElement('span'));
            tds[3].childNodes[1].classList.add('mdl-typography--font-light');
            tds[3].childNodes[1].appendChild(document.createTextNode(eventAction.request.content1 + " "));
            tds[3].appendChild(document.createElement('span'));
            tds[3].childNodes[2].classList.add('mdl-typography--font-light');
            tds[3].childNodes[2].appendChild(document.createTextNode(eventAction.request.content2));
        }

        for (const td of Object.values(tds)) {
            tr.appendChild(td);
        }
        $eventActionTable.appendChild(tr);
    }
}, 50, {'leading': false, 'trailing': true});

            events[id]["request"] = request;

            createEventActionTable();
        }
        if (checkListRaw) {
            var checkList = checkListRaw.engValue.arrayValue;

            for (var checkRaw of checkList) {
                var check = findkey(checkRaw.aggregateValue);

                var transition = {
                    "pmonId": check("PMON_ID").uint32Value,
                    "parameter": Parameters[check("Monitored_Parameter_ID").uint32Value],
                    "check_type": check("Check_Type").uint32Value,
                    "value": check("Value").floatValue,
                    "previous": CheckStatusEnumeration[check("Previous_Check_Status").uint32Value],
                    "current": CheckStatusEnumeration[check("Current_Check_Status").uint32Value],
                    "timestamp": check("Timestamp").uint32Value
                }

                transitions.push(transition);
            }

            createTransitionTable();
            getST12definitions();
        }
        if (monitoringRaw) {
            var checkRaw = _.find(json.data.values, function(e) { return e.numericId != 1 });

            monitoring = findkey(monitoringRaw.engValue.aggregateValue);
            check = findkey(checkRaw.engValue.aggregateValue);

            if (check("Mask") !== undefined) {
                checkData = {
                    "mask": "0x" + check("Mask").uint32Value.toString(16),
                    "value": check("Expected_Value").stringValue 
                        ? check("Expected_Value").stringValue
                        : check("Expected_Value").uint32Value,
                    "event": check("Event_Definition_ID").stringValue
                }
            } else {
                checkData = {
                    "low": check("Low_Limit").floatValue,
                    "low_event": check("Low_Event").stringValue,
                    "high": check("High_Limit").floatValue,
                    "high_event": check("High_Event").stringValue
                }
            }

            pmons[monitoring("PMON_ID").uint32Value] = {
                "parameter": Parameters[monitoring("Monitored_Parameter_ID").uint32Value],
                "validity": Parameters[monitoring("Mask").uint32Value] == 0 ? null : {
                    "parameter": Parameters[monitoring("Validity_Parameter_ID").uint32Value],
                    "mask": "0x" + monitoring("Mask").uint32Value.toString(16),
                    "value": monitoring("Expected_Value").uint32Value,
                },
                "monitoring_interval": monitoring("Monitoring_Interval").uint32Value + " ms",
                "status": monitoring("Check_Status").stringValue,
                "repetition_number": monitoring("Repetition_Number").uint32Value,
                "check": checkData,
                "check_type": monitoring("Check_Type").stringValue,
                "date": monitoringRaw.generationTime
            }

            createPmonTable();
        }
    }
}
