// BLE Service and Characteristics UUIDs
const SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
const CHAR_JSON_GATE_UUID = "a1b2c3d4-1234-5678-9abc-def012345685";

// Register Service Worker for PWA
if ('serviceWorker' in navigator) {
    window.addEventListener('load', () => {
        navigator.serviceWorker.register('./sw.js')
            .then(reg => console.log('[PWA] Service Worker registered'))
            .catch(err => console.log('[PWA] Service Worker failed', err));
    });
}

let device, server, service;
let characteristics = {};
let rxBuffer = new Uint8Array(0);
let rxExpectedLen = 0;
let rxExpectedType = 0;
let rxState = 'HEADER';
let sensorDataResolver = null;
let configDataResolver = null;

// DOM Elements
let currentSelectedSlot = 0; // State tracking
const connectBtn = document.getElementById('connectBtn');
const connectionStatus = document.getElementById('connectionStatus');
const connectScreen = document.getElementById('connectScreen');
const tabNav = document.getElementById('tabNav');
const tabs = document.querySelectorAll('.tab');

// Internationalization
let currentLang = localStorage.getItem('lang') || 'en';

function updateLanguage() {
    document.querySelectorAll('[data-i18n]').forEach(element => {
        const key = element.getAttribute('data-i18n');
        if (translations[currentLang] && translations[currentLang][key]) {
            if (element.tagName === 'INPUT' && element.type === 'placeholder') {
                element.placeholder = translations[currentLang][key];
            } else {
                element.textContent = translations[currentLang][key];
            }
        }
    });
    const selector = document.getElementById('languageSelector');
    if (selector) selector.value = currentLang;
    localStorage.setItem('lang', currentLang);
}

function handleRouting() {
    const hash = window.location.hash.substring(1) || 'connect';
    const protectedTabs = ['network', 'system', 'events', 'sensors'];

    if (protectedTabs.includes(hash) && (!device || !device.gatt.connected)) {
        window.location.hash = '#connect';
        return;
    }

    showTab(hash);
}

window.addEventListener('hashchange', handleRouting);
document.addEventListener('DOMContentLoaded', () => {
    updateLanguage();
    handleRouting();
    const langSelector = document.getElementById('languageSelector');
    if (langSelector) {
        langSelector.addEventListener('change', (e) => {
            currentLang = e.target.value;
            updateLanguage();
        });
    }
});

// Render Sensor Grid (16 slots)
function renderSensorGrid() {
    const container = document.getElementById('sensorQuickView');
    if (!container) return;
    container.innerHTML = '';
    for (let i = 0; i < 16; i++) {
        const div = document.createElement('div');
        div.className = 'quick-view-item';
        if (i === currentSelectedSlot) div.classList.add('active');
        div.innerHTML = `Slot ${i} <br><span id="qv${i}">--</span>`;
        div.onclick = () => selectSlot(i);
        container.appendChild(div);
    }
}
renderSensorGrid();

// Tab Navigation
tabs.forEach(tab => {
    tab.addEventListener('click', () => {
        const targetTab = tab.dataset.tab;
        showTab(targetTab);
    });
});

window.showTab = function (tabId) {
    // If it's a "screen" but not "tab-content", handle separately if needed
    // or just assume all are tab-contents now
    const screens = document.querySelectorAll('.screen, .tab-content');
    screens.forEach(el => el.classList.add('hidden'));

    const targetId = tabId === 'connect' ? 'connectScreen' : `tab-${tabId}`;
    const selected = document.getElementById(targetId);
    if (selected) selected.classList.remove('hidden');

    // Update Tab UI
    tabs.forEach(t => t.classList.remove('active'));
    const btn = document.querySelector(`.tab[data-tab="${tabId}"]`);
    if (btn) btn.classList.add('active');

    // Sync Hash
    if (window.location.hash !== `#${tabId}`) {
        window.location.hash = tabId;
    }

    // Hide/Show Nav based on connection
    const isConnected = (device && device.gatt.connected);
    tabNav.style.display = isConnected ? 'flex' : 'none';
};

// BLE Logic
function prepareBleJson(obj) {
    const json = JSON.stringify(obj);
    const encoder = new TextEncoder();
    const payload = encoder.encode(json);
    const header = new Uint8Array([0x00, payload.length & 0xFF, (payload.length >> 8) & 0xFF]);
    const bytes = new Uint8Array(header.length + payload.length);
    bytes.set(header);
    bytes.set(payload, header.length);
    return bytes;
}

async function sendCommand(cmdObj) {
    if (!characteristics.gate) {
        console.error("Not connected");
        return;
    }

    const bytes = prepareBleJson(cmdObj);
    console.log(`[BLE] Sending command: ${JSON.stringify(cmdObj)} (Total bytes: ${bytes.length})`);

    const MTU = 100; // Small chunks for reliability
    for (let i = 0; i < bytes.length; i += MTU) {
        const chunk = bytes.slice(i, i + MTU);
        await characteristics.gate.writeValue(chunk);
    }
}

let fullStore = {};

function handleIncomingJson(data) {
    console.log("[BLE] Received JSON:", data);
    if (data.cmd === "config_data") {
        fullStore = data;

        // Sync UI with fullStore
        if (data.ssid !== undefined) document.getElementById('ssid').value = data.ssid;
        if (data.ntpServer !== undefined) document.getElementById('ntpServer').value = data.ntpServer;
        if (data.gmt !== undefined) document.getElementById('gmtOffset').value = data.gmt;
        if (data.dst !== undefined) document.getElementById('dstOffset').value = data.dst;
        if (data.dnsMode !== undefined) {
            const radio = document.querySelector(`input[name="dnsMode"][value="${data.dnsMode}"]`);
            if (radio) radio.checked = true;
            document.getElementById('manualDnsFields').classList.toggle('hidden', data.dnsMode !== 'manual');
        }
        if (data.dnsPrimary !== undefined) document.getElementById('dnsPrimary').value = data.dnsPrimary;
        if (data.dnsSecondary !== undefined) document.getElementById('dnsSecondary').value = data.dnsSecondary;
        if (data.tempusUrl !== undefined) document.getElementById('tempusUrl').value = data.tempusUrl;
        if (data.bleTimeout !== undefined) document.getElementById('bleTimeout').value = data.bleTimeout;
        if (data.sensorInterval !== undefined) document.getElementById('sensorInterval').value = data.sensorInterval;
        if (data.style !== undefined) document.getElementById('styleSelector').value = data.style;
        if (data.lang !== undefined) {
            currentLang = data.lang;
            updateLanguage();
        }

        // Update Sensor Quick View
        if (Array.isArray(data.sensors)) {
            data.sensors.forEach((s, idx) => {
                const sensor = s || {};
                updateQuickView(idx, sensor.enabled, sensor.label, sensor.unit);
            });
        }

        if (configDataResolver) {
            configDataResolver.resolve();
            configDataResolver = null;
        }
    } else if (data.cmd === "save_ok") {
        showStatus(translations[currentLang].status_saved, false);
    }
}

function onNotification(event) {
    // .slice() creates a COPY of the buffer. Without it, rapid
    // indications can overwrite the buffer before JS processes the event.
    const rawBuf = event.target.value;
    const value = new Uint8Array(rawBuf.buffer.slice(rawBuf.byteOffset, rawBuf.byteOffset + rawBuf.byteLength));
    if (value.length === 0) {
        console.warn("[BLE] Skipping empty notification");
        return;
    }
    console.log(`[BLE] Received chunk: ${value.length} bytes`);

    // Append to rxBuffer
    const nextBuffer = new Uint8Array(rxBuffer.length + value.length);
    nextBuffer.set(rxBuffer);
    nextBuffer.set(value, rxBuffer.length);
    rxBuffer = nextBuffer;

    while (rxBuffer.length > 0) {
        if (rxState === 'HEADER') {
            if (rxBuffer.length >= 3) {
                const type = rxBuffer[0];
                const len = rxBuffer[1] | (rxBuffer[2] << 8);

                if (type !== 0x00 || len > 5000 || len === 0) {
                    console.warn(`[BLE] Invalid header: Type=${type}, Len=${len}. Searching for next 0x00.`);
                    // Search for next potential header (Type=0x00)
                    let nextHeaderIdx = -1;
                    for (let i = 1; i < rxBuffer.length; i++) {
                        if (rxBuffer[i] === 0x00) {
                            nextHeaderIdx = i;
                            break;
                        }
                    }
                    if (nextHeaderIdx !== -1) {
                        rxBuffer = rxBuffer.slice(nextHeaderIdx);
                        continue; // Retry header parsing
                    } else {
                        // No 0x00 found, keep last byte in case it's part of a header
                        rxBuffer = rxBuffer.slice(rxBuffer.length - 1);
                        break;
                    }
                }

                rxExpectedType = type;
                rxExpectedLen = len;
                rxBuffer = rxBuffer.slice(3);
                rxState = 'PAYLOAD';
                console.log(`[BLE] Header valid: Type=${rxExpectedType}, Len=${rxExpectedLen}`);
            } else {
                break;
            }
        }

        if (rxState === 'PAYLOAD') {
            if (rxBuffer.length >= rxExpectedLen) {
                const payload = rxBuffer.slice(0, rxExpectedLen);
                rxBuffer = rxBuffer.slice(rxExpectedLen);
                rxState = 'HEADER';

                if (rxExpectedType === 0x00) { // JSON Mode
                    try {
                        const decoder = new TextDecoder();
                        const jsonStr = decoder.decode(payload);
                        console.log(`[BLE] Full JSON received: ${jsonStr}`);
                        const data = JSON.parse(jsonStr);
                        handleIncomingJson(data);
                    } catch (e) {
                        console.error("[BLE] JSON Parse Error:", e);
                        console.error("[BLE] Raw Payload:", payload);
                    }
                } else {
                    console.log(`[BLE] Received Binary Type ${rxExpectedType}, len ${rxExpectedLen}`);
                }
            } else {
                console.log(`[BLE] Waiting for payload: ${rxBuffer.length}/${rxExpectedLen}`);
                break; // Need more bytes for payload
            }
        }
    }
}

// Connection
connectBtn.addEventListener('click', async () => {
    const spinner = document.getElementById('connectSpinner');
    const btn = document.getElementById('connectBtn');
    try {
        btn.classList.add('hidden');
        spinner.classList.remove('hidden');
        showStatus(translations[currentLang].status_connecting, false);

        device = await navigator.bluetooth.requestDevice({
            filters: [{ services: [SERVICE_UUID] }]
        });
        device.addEventListener('gattserverdisconnected', onDisconnected);
        server = await device.gatt.connect();
        service = await server.getPrimaryService(SERVICE_UUID);

        characteristics.gate = await service.getCharacteristic(CHAR_JSON_GATE_UUID);
        await characteristics.gate.startNotifications();
        characteristics.gate.addEventListener('characteristicvaluechanged', onNotification);

        // RESET BLE STATE ON NEW CONNECTION
        rxBuffer = new Uint8Array(0);
        rxState = 'HEADER';
        console.log("[BLE] State reset for new connection.");

        // ⏱️ CRITICAL: Wait for BLE stack to be ready before sending commands
        await new Promise(resolve => setTimeout(resolve, 200));
        console.log("[BLE] BLE stack ready, proceeding...");

        // UI state update
        connectionStatus.textContent = translations[currentLang].status_connected;
        connectionStatus.classList.remove('disconnected');
        connectionStatus.classList.add('connected');
        document.getElementById('disconnectBtn').classList.remove('hidden');

        window.location.hash = '#network'; // Redirect to main app on connect
        showStatus(translations[currentLang].status_connected_msg, false);

        // Load initial settings and WAIT for response
        console.log("[BLE] Loading initial config...");
        await new Promise((resolve) => {
            configDataResolver = { resolve };
            sendCommand({ cmd: "get_config" }).catch(() => {
                configDataResolver = null;
                resolve();
            });
            // 2s Timeout
            setTimeout(() => { if (configDataResolver) { configDataResolver = null; resolve(); } }, 2000);
        });

        // Load all quick views AFTER config is done
        await loadAllSensorsQuickView();
    } catch (error) {
        console.error('Connection failed:', error);
        showStatus((translations[currentLang].status_connection_failed) + error.message, true);
    } finally {
        spinner.classList.add('hidden');
        btn.classList.remove('hidden');
    }
});

function onDisconnected() {
    connectionStatus.textContent = translations[currentLang].status_disconnected;
    connectionStatus.classList.add('disconnected');
    connectionStatus.classList.remove('connected');
    document.getElementById('disconnectBtn').classList.add('hidden');
    window.location.hash = '#connect';
    document.querySelectorAll('.tab-content').forEach(c => c.classList.add('hidden'));
    showStatus('Device disconnected', true);
}

document.getElementById('disconnectBtn').addEventListener('click', () => {
    if (device && device.gatt.connected) device.gatt.disconnect();
});

// Consolidated Global Save
async function saveFullConfig(partialUpdate = {}) {
    const config = {
        ssid: document.getElementById('ssid').value,
        ntpServer: document.getElementById('ntpServer').value,
        gmt: parseInt(document.getElementById('gmtOffset').value),
        dst: parseInt(document.getElementById('dstOffset').value),
        dnsMode: document.querySelector('input[name="dnsMode"]:checked').value,
        dnsPrimary: document.getElementById('dnsPrimary').value,
        dnsSecondary: document.getElementById('dnsSecondary').value,
        tempusUrl: document.getElementById('tempusUrl').value,
        bleTimeout: parseInt(document.getElementById('bleTimeout').value),
        sensorInterval: parseInt(document.getElementById('sensorInterval').value),
        style: parseInt(document.getElementById('styleSelector').value),
        lang: document.getElementById('languageSelector').value,
        module_map: fullStore.module_map || "",
        // Sensors are kept from fullStore unless updated
        sensors: fullStore.sensors || [],
        ...partialUpdate
    };

    // Include wifi password only if provided
    const pass = document.getElementById('password').value;
    if (pass) config.password = pass;

    try {
        await sendCommand({ cmd: "save_config", config });
        // Update local store immediately for UI consistency
        Object.assign(fullStore, config);
    } catch (e) {
        showStatus(translations[currentLang].status_save_failed + e.message, true);
    }
}

// Settings Handlers
document.getElementById('saveWifiBtn').addEventListener('click', () => saveFullConfig());
document.getElementById('saveDnsBtn').addEventListener('click', () => saveFullConfig());
document.getElementById('saveNtpBtn').addEventListener('click', () => saveFullConfig());
document.getElementById('saveTempusUrlBtn').addEventListener('click', () => saveFullConfig());

document.getElementById('saveSensorBtn').addEventListener('click', async () => {
    const slot = currentSelectedSlot;
    if (!fullStore.sensors) fullStore.sensors = Array(16).fill({});

    fullStore.sensors[slot] = {
        label: document.getElementById('sensorLabel').value,
        url: document.getElementById('sensorUrl').value,
        unit: document.getElementById('sensorUnit').value,
        divisor: parseFloat(document.getElementById('sensorDivisor').value),
        decimals: parseInt(document.getElementById('sensorDecimals').value),
        enabled: document.getElementById('sensorEnabled').checked,
        type: document.getElementById('sensorType').value,
        jsonPath: document.getElementById('sensorJsonPath').value
    };

    await saveFullConfig();
    updateQuickView(slot, fullStore.sensors[slot].enabled, fullStore.sensors[slot].label, fullStore.sensors[slot].unit);
});

document.getElementById('clearSensorBtn').addEventListener('click', async () => {
    const slot = currentSelectedSlot;
    if (!fullStore.sensors) fullStore.sensors = Array(16).fill({});

    fullStore.sensors[slot] = { label: "", url: "", unit: "", divisor: 1, decimals: 1, enabled: false, type: "prometheus", jsonPath: "" };

    await saveFullConfig();
    updateQuickView(slot, false, "", "");
    // Reset form
    document.getElementById('sensorLabel').value = "";
    document.getElementById('sensorEnabled').checked = false;
});

document.getElementById('saveSysConfigBtn').addEventListener('click', () => saveFullConfig());
const saveGlobalBtn = document.getElementById('saveGlobalSensorBtn');
if (saveGlobalBtn) saveGlobalBtn.addEventListener('click', () => saveFullConfig());

async function syncUIWithSensor(slot) {
    const s = (fullStore.sensors && fullStore.sensors[slot]) || {};
    document.getElementById('sensorLabel').value = s.label || "";
    document.getElementById('sensorUrl').value = s.url || "";
    document.getElementById('sensorUnit').value = s.unit || "";
    document.getElementById('sensorDivisor').value = s.divisor || "1";
    document.getElementById('sensorDecimals').value = s.decimals || "1";
    document.getElementById('sensorEnabled').checked = s.enabled || false;
    document.getElementById('sensorType').value = s.type || "prometheus";
    document.getElementById('sensorJsonPath').value = s.jsonPath || "";
    document.getElementById('sensorType').dispatchEvent(new Event('change'));
}

async function loadAllSensorsQuickView() {
    // Already loaded via get_config on connection
}

window.selectSlot = async function (slot) {
    currentSelectedSlot = slot;
    document.querySelectorAll('.quick-view-item').forEach((el, i) => {
        el.classList.toggle('selected', i === slot);
    });
    document.getElementById('configTitle').textContent = `Configuration: Slot ${slot}`;
    syncUIWithSensor(slot);
    document.getElementById('sensorConfigCard').scrollIntoView({ behavior: 'smooth' });
};

function updateQuickView(slot, enabled, label, unit) {
    const qv = document.getElementById(`qv${slot}`);
    if (qv) {
        qv.textContent = (enabled && label) ? `✓ ${label} (${unit})` : "✗ Not configured";
        qv.parentElement.style.opacity = (enabled && label) ? "1" : "0.5";
    }
}

function showStatus(message, isError) {
    const el = document.getElementById('statusMessage');
    el.textContent = message;
    el.className = `status-message show ${isError ? 'error' : ''}`;
    setTimeout(() => el.classList.remove('show'), 3000);
}

// UI Toggles
document.querySelectorAll('input[name="dnsMode"]').forEach(r => {
    r.addEventListener('change', (e) => {
        document.getElementById('manualDnsFields').classList.toggle('hidden', e.target.value !== 'manual');
    });
});
document.getElementById('sensorType').addEventListener('change', (e) => {
    document.getElementById('jsonPathGroup').classList.toggle('hidden', e.target.value !== 'json');
});
