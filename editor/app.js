const state = {
  project: "",
  file: "",
  model: { states: [], instances: [], hooks: [] },
  standardStates: [],
  selected: null,
  source: "",
};

const elements = {
  projectPath: document.getElementById("projectPath"),
  schemaSelect: document.getElementById("schemaSelect"),
  reloadButton: document.getElementById("reloadButton"),
  saveButton: document.getElementById("saveButton"),
  generateButton: document.getElementById("generateButton"),
  addStateButton: document.getElementById("addStateButton"),
  standardStateSelect: document.getElementById("standardStateSelect"),
  addStandardStateButton: document.getElementById("addStandardStateButton"),
  addInstanceButton: document.getElementById("addInstanceButton"),
  addHookButton: document.getElementById("addHookButton"),
  autoLayoutButton: document.getElementById("autoLayoutButton"),
  deleteButton: document.getElementById("deleteButton"),
  graph: document.getElementById("graph"),
  inspectorTitle: document.getElementById("inspectorTitle"),
  inspectorBody: document.getElementById("inspectorBody"),
  sourcePreview: document.getElementById("sourcePreview"),
  status: document.getElementById("status"),
};

let graphView = null;
const graphPositions = new Map();
const graphNodeIds = new WeakMap();
let nextGraphNodeId = 1;

function setStatus(message, isError = false) {
  elements.status.textContent = message;
  elements.status.classList.toggle("error", isError);
}

async function requestJson(url, options = {}) {
  const response = await fetch(url, options);
  const payload = await response.json();
  if (!response.ok) {
    throw new Error(payload.error || "request failed");
  }
  return payload;
}

function uniqueName(prefix, names) {
  let index = names.length + 1;
  let candidate = `${prefix}${index}`;
  while (names.includes(candidate)) {
    index += 1;
    candidate = `${prefix}${index}`;
  }
  return candidate;
}

function stateNames() {
  return state.model.states.map((item) => item.name);
}

function selectedState() {
  if (!state.selected || state.selected.type !== "state") {
    return null;
  }
  return state.model.states[state.selected.index] || null;
}

function selectedHook() {
  if (!state.selected || state.selected.type !== "hook") {
    return null;
  }
  return state.model.hooks[state.selected.index] || null;
}

function selectedInstance() {
  if (!state.selected || state.selected.type !== "instance") {
    return null;
  }
  return state.model.instances[state.selected.index] || null;
}

function sanitizeModel() {
  state.model.version = state.model.version || 1;
  state.model.states = state.model.states || [];
  state.model.instances = state.model.instances || [];
  state.model.hooks = state.model.hooks || [];
  for (const item of state.model.states) {
    item.name = item.name || "State";
    item.fields = item.fields || [];
    for (const field of item.fields) {
      field.name = field.name || "field";
      field.type = field.type || "i32";
      if (field.default === undefined) {
        field.default = defaultForType(field.type);
      }
    }
    item.constructors = item.constructors || [];
  }
  for (const hook of state.model.hooks) {
    hook.name = hook.name || "Hook";
    hook.on = hook.on || {};
    hook.on.state = hook.on.state || state.model.states[0]?.name || "";
    hook.on.event = "changed";
    hook.reads = hook.reads || [];
    hook.writes = hook.writes || [];
  }
  for (const instance of state.model.instances) {
    instance.name = instance.name || "instance";
    instance.state = instance.state || state.model.states[0]?.name || "";
  }
}

function renderLocalSource() {
  return `${JSON.stringify(state.model, null, 2)}\n`;
}

function defaultForType(type) {
  if (type === "String") return "";
  if (type === "bool") return false;
  return 0;
}

function parseEditorValue(type, value) {
  if (type === "String") return value;
  if (type === "bool") return value === "true";
  if (value === "") return 0;
  return Number(value);
}

function valueForInput(value) {
  if (value === undefined || value === null) return "";
  return String(value);
}

async function refreshPreview() {
  sanitizeModel();
  state.source = renderLocalSource();
  elements.sourcePreview.value = state.source;
  try {
    const payload = await requestJson("/api/render", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ source: state.source }),
    });
    state.source = payload.source;
    setStatus("Valid schema");
  } catch (error) {
    setStatus(error.message, true);
  }
}

function graphNodeId(type, item) {
  if (!graphNodeIds.has(item)) {
    graphNodeIds.set(item, `${type}:${nextGraphNodeId}`);
    nextGraphNodeId += 1;
  }
  return graphNodeIds.get(item);
}

function defaultPosition(type, index) {
  if (type === "state") {
    return { x: 60 + (index % 3) * 280, y: 70 + Math.floor(index / 3) * 170 };
  }
  return { x: 900, y: 70 + index * 150 };
}

function rememberGraphPositions() {
  if (!graphView) {
    return;
  }
  graphView.nodes().forEach((node) => {
    graphPositions.set(node.id(), { ...node.position() });
  });
}

function graphElements() {
  const elements = [];
  const stateIds = new Map();
  const hookIds = new Map();
  const instanceIds = new Map();

  state.model.states.forEach((item, index) => {
    const id = graphNodeId("state", item);
    const position = graphPositions.get(id) || defaultPosition("state", index);
    if (!stateIds.has(item.name)) {
      stateIds.set(item.name, id);
    }
    const meta = `${item.fields.length} fields, ${item.constructors.length} constructors`;
    elements.push({
      data: {
        id,
        type: "state",
        index,
        label: item.name,
        displayLabel: `${item.name}\n${meta}`,
        meta,
      },
      position,
      classes: state.selected?.type === "state" && state.selected.index === index ? "selected" : "",
    });
  });

  state.model.hooks.forEach((hook, index) => {
    const id = graphNodeId("hook", hook);
    const position = graphPositions.get(id) || defaultPosition("hook", index);
    if (!hookIds.has(hook.name)) {
      hookIds.set(hook.name, id);
    }
    const meta = `on ${hook.on?.state}.changed`;
    elements.push({
      data: {
        id,
        type: "hook",
        index,
        label: hook.name,
        displayLabel: `${hook.name}\n${meta}`,
        meta,
      },
      position,
      classes: state.selected?.type === "hook" && state.selected.index === index ? "selected" : "",
    });
  });

  state.model.instances.forEach((instance, index) => {
    const id = graphNodeId("instance", instance);
    const position = graphPositions.get(id) || { x: 60 + (index % 3) * 280, y: 580 + Math.floor(index / 3) * 140 };
    instanceIds.set(instance.name, id);
    const meta = `instance of ${instance.state}`;
    elements.push({
      data: {
        id,
        type: "instance",
        index,
        label: instance.name,
        displayLabel: `${instance.name}\n${meta}`,
        meta,
      },
      position,
      classes: state.selected?.type === "instance" && state.selected.index === index ? "selected" : "",
    });
  });

  state.model.instances.forEach((instance, index) => {
    const stateId = stateIds.get(instance.state);
    const instanceId = instanceIds.get(instance.name);
    if (stateId && instanceId) {
      elements.push(edgeElement(`instance:${index}:${instance.state}`, stateId, instanceId, "instance", "instance"));
    }
  });

  state.model.hooks.forEach((hook, hookIndex) => {
    const hookId = hookIds.get(hook.name);
    const onStateId = stateIds.get(hook.on?.state);
    if (onStateId && hookId) {
      elements.push(edgeElement(`changed:${hookIndex}:${hook.on.state}`, onStateId, hookId, "changed", "changed"));
    }
    hook.reads.forEach((name, readIndex) => {
      const stateId = stateIds.get(name);
      if (stateId && hookId) {
        elements.push(edgeElement(`reads:${hookIndex}:${readIndex}:${name}`, stateId, hookId, "reads", "reads"));
      }
    });
    hook.writes.forEach((name, writeIndex) => {
      const stateId = stateIds.get(name);
      if (hookId && stateId) {
        elements.push(edgeElement(`writes:${hookIndex}:${writeIndex}:${name}`, hookId, stateId, "writes", "writes"));
      }
    });
  });

  return elements;
}

function edgeElement(id, source, target, label, type) {
  return {
    data: { id, source, target, label, type },
    classes: type,
  };
}

function graphStyles() {
  return [
    {
      selector: "node",
      style: {
        "background-color": "#172033",
        "border-color": "#334155",
        "border-width": 1,
        "color": "#edf2ff",
        "content": "data(displayLabel)",
        "font-size": 15,
        "font-weight": 800,
        "height": 86,
        "label": "data(displayLabel)",
        "padding": 14,
        "shape": "round-rectangle",
        "text-halign": "center",
        "text-valign": "center",
        "text-wrap": "wrap",
        "width": 216,
      },
    },
    {
      selector: "node[type = 'hook']",
      style: {
        "background-color": "#281a3f",
        "border-style": "double",
        "border-width": 3,
        "shape": "barrel",
      },
    },
    {
      selector: "node[type = 'instance']",
      style: {
        "background-color": "#14342f",
        "border-color": "#2dd4bf",
      },
    },
    {
      selector: "edge",
      style: {
        "curve-style": "bezier",
        "color": "#f8fafc",
        "font-size": 12,
        "font-weight": 800,
        "label": "data(label)",
        "line-color": "#94a3b8",
        "target-arrow-color": "#94a3b8",
        "target-arrow-shape": "triangle",
        "text-background-color": "#020617",
        "text-background-opacity": 1,
        "text-background-padding": 5,
        "text-border-color": "#475569",
        "text-border-opacity": 1,
        "text-border-width": 1,
        "text-margin-y": -10,
        "width": 2,
      },
    },
    {
      selector: "edge.changed",
      style: {
        "line-color": "#5eead4",
        "target-arrow-color": "#5eead4",
      },
    },
    {
      selector: "edge.reads",
      style: {
        "line-color": "#facc15",
        "line-style": "dashed",
        "target-arrow-color": "#facc15",
      },
    },
    {
      selector: "edge.writes",
      style: {
        "line-color": "#c084fc",
        "target-arrow-color": "#c084fc",
      },
    },
    {
      selector: "edge.instance",
      style: {
        "line-color": "#2dd4bf",
        "line-style": "dotted",
        "target-arrow-color": "#2dd4bf",
      },
    },
    {
      selector: "node.highlighted",
      style: {
        "border-color": "#7dd3fc",
        "border-width": 3,
      },
    },
    {
      selector: "edge.highlighted",
      style: {
        "opacity": 1,
        "width": 4,
      },
    },
    {
      selector: "node.selected",
      style: {
        "border-color": "#38bdf8",
        "border-width": 4,
      },
    },
    {
      selector: "edge.selected",
      style: {
        "opacity": 1,
        "width": 4,
      },
    },
    {
      selector: ".dimmed",
      style: {
        "opacity": 0.25,
      },
    },
  ];
}

function initGraph() {
  if (graphView) {
    return;
  }
  if (!window.cytoscape) {
    elements.graph.textContent = "Graph library failed to load.";
    setStatus("Graph library failed to load", true);
    return;
  }

  graphView = cytoscape({
    container: elements.graph,
    elements: [],
    layout: { name: "preset" },
    maxZoom: 2.2,
    minZoom: 0.35,
    style: graphStyles(),
  });

  graphView.on("tap", "node", (event) => {
    const data = event.target.data();
    selectItem(data.type, data.index);
  });

  graphView.on("tap", "edge", (event) => {
    clearGraphSelectionClasses();
    event.target.addClass("selected");
    highlightConnected(event.target);
  });

  graphView.on("tap", (event) => {
    if (event.target === graphView) {
      state.selected = null;
      renderGraph();
      renderInspector();
    }
  });

  graphView.on("dragfree", "node", () => rememberGraphPositions());
}

function renderGraph() {
  initGraph();
  if (!graphView) {
    return;
  }
  rememberGraphPositions();
  graphView.batch(() => {
    graphView.elements().remove();
    graphView.add(graphElements());
    graphView.layout({ name: "preset", fit: false }).run();
  });
  highlightSelectedGraphItem();
}

function clearGraphSelectionClasses() {
  if (!graphView) {
    return;
  }
  graphView.elements().removeClass("selected highlighted dimmed");
}

function highlightConnected(item) {
  if (!graphView) {
    return;
  }
  const connected = item.isNode() ? item.closedNeighborhood() : item.connectedNodes().union(item);
  graphView.elements().difference(connected).addClass("dimmed");
  connected.addClass("highlighted");
  item.addClass("selected");
}

function highlightSelectedGraphItem() {
  if (!graphView) {
    return;
  }
  clearGraphSelectionClasses();
  if (!state.selected) {
    return;
  }
  const selector = `node[type = '${state.selected.type}'][index = ${state.selected.index}]`;
  const node = graphView.$(selector);
  if (node.nonempty()) {
    highlightConnected(node);
  }
}

function runAutoLayout() {
  initGraph();
  if (!graphView) {
    return;
  }
  graphView.one("layoutstop", () => rememberGraphPositions());
  const layout = graphView.layout({
    name: "breadthfirst",
    directed: true,
    fit: true,
    padding: 48,
    spacingFactor: 1.25,
  });
  layout.run();
}

function input(label, value, onChange) {
  const group = document.createElement("div");
  group.className = "field-group";
  const labelElement = document.createElement("label");
  labelElement.textContent = label;
  const inputElement = document.createElement("input");
  inputElement.value = value || "";
  inputElement.addEventListener("input", () => onChange(inputElement.value));
  group.append(labelElement, inputElement);
  return group;
}

function select(label, value, options, onChange) {
  const group = document.createElement("div");
  group.className = "field-group";
  const labelElement = document.createElement("label");
  labelElement.textContent = label;
  const selectElement = document.createElement("select");
  for (const option of options) {
    const item = document.createElement("option");
    item.value = option;
    item.textContent = option;
    item.selected = option === value;
    selectElement.appendChild(item);
  }
  selectElement.addEventListener("change", () => onChange(selectElement.value));
  group.append(labelElement, selectElement);
  return group;
}

function actionButton(text, onClick, className = "") {
  const button = document.createElement("button");
  button.type = "button";
  button.textContent = text;
  button.className = className;
  button.addEventListener("click", onClick);
  return button;
}

function rerenderAll() {
  renderGraph();
  renderInspector();
  refreshPreview();
}

function renderStateInspector(item) {
  elements.inspectorTitle.textContent = `State: ${item.name}`;
  elements.inspectorBody.innerHTML = "";
  elements.inspectorBody.appendChild(input("Name", item.name, (value) => {
    const oldName = item.name;
    item.name = value;
    for (const hook of state.model.hooks) {
      hook.on = hook.on || {};
      if (hook.on.state === oldName) hook.on.state = value;
      hook.reads = hook.reads.map((name) => (name === oldName ? value : name));
      hook.writes = hook.writes.map((name) => (name === oldName ? value : name));
    }
    rerenderAll();
  }));

  const fieldsTitle = document.createElement("div");
  fieldsTitle.className = "list-title";
  fieldsTitle.textContent = "Fields";
  elements.inspectorBody.appendChild(fieldsTitle);
  item.fields.forEach((field, index) => {
    const card = document.createElement("div");
    card.className = "list-card";
    const grid = document.createElement("div");
    grid.className = "grid-two";
    grid.appendChild(input("Name", field.name, (value) => {
      const oldName = field.name;
      field.name = value;
      for (const constructor of item.constructors) {
        if (constructor.values && constructor.values[oldName] !== undefined) {
          constructor.values[value] = constructor.values[oldName];
          delete constructor.values[oldName];
        }
      }
      rerenderAll();
    }));
    grid.appendChild(input("Type", field.type, (value) => {
      field.type = value;
      field.default = defaultForType(value);
      rerenderAll();
    }));
    card.appendChild(grid);
    card.appendChild(input("Default", valueForInput(field.default), (value) => {
      field.default = parseEditorValue(field.type, value);
      rerenderAll();
    }));
    const constraints = document.createElement("div");
    constraints.className = "grid-two";
    constraints.appendChild(input("Min", valueForInput(field.min), (value) => {
      if (value === "") {
        delete field.min;
      } else {
        field.min = Number(value);
      }
      rerenderAll();
    }));
    constraints.appendChild(input("Max", valueForInput(field.max), (value) => {
      if (value === "") {
        delete field.max;
      } else {
        field.max = Number(value);
      }
      rerenderAll();
    }));
    card.appendChild(constraints);
    card.appendChild(actionButton("Remove Field", () => {
      item.fields.splice(index, 1);
      for (const constructor of item.constructors) {
        delete constructor.values[field.name];
      }
      rerenderAll();
    }, "danger"));
    elements.inspectorBody.appendChild(card);
  });
  elements.inspectorBody.appendChild(actionButton("Add Field", () => {
    const name = uniqueName("field", item.fields.map((field) => field.name));
    item.fields.push({ name, type: "i32", default: 0 });
    rerenderAll();
  }));

  const constructorsTitle = document.createElement("div");
  constructorsTitle.className = "list-title";
  constructorsTitle.textContent = "Constructors";
  elements.inspectorBody.appendChild(constructorsTitle);
  item.constructors.forEach((constructor, index) => {
    const card = document.createElement("div");
    card.className = "list-card";
    constructor.values = constructor.values || {};
    card.appendChild(input("Name", constructor.name, (value) => {
      constructor.name = value;
      rerenderAll();
    }));
    for (const field of item.fields) {
      card.appendChild(input(`${field.name} override`, valueForInput(constructor.values[field.name]), (value) => {
        if (value === "") {
          delete constructor.values[field.name];
        } else {
          constructor.values[field.name] = parseEditorValue(field.type, value);
        }
        rerenderAll();
      }));
    }
    card.appendChild(actionButton("Remove Constructor", () => {
      item.constructors.splice(index, 1);
      rerenderAll();
    }, "danger"));
    elements.inspectorBody.appendChild(card);
  });
  elements.inspectorBody.appendChild(actionButton("Add Constructor", () => {
    item.constructors.push({ name: uniqueName("constructor", item.constructors.map((constructor) => constructor.name)), values: {} });
    rerenderAll();
  }));
}

function checkboxList(title, selected, onChange) {
  const wrap = document.createElement("div");
  const titleElement = document.createElement("div");
  titleElement.className = "list-title";
  titleElement.textContent = title;
  const list = document.createElement("div");
  list.className = "check-list";
  for (const name of stateNames()) {
    const row = document.createElement("label");
    row.className = "check-row";
    const box = document.createElement("input");
    box.type = "checkbox";
    box.checked = selected.includes(name);
    box.addEventListener("change", () => {
      const next = new Set(selected);
      if (box.checked) {
        next.add(name);
      } else {
        next.delete(name);
      }
      onChange(Array.from(next));
    });
    row.append(box, document.createTextNode(name));
    list.appendChild(row);
  }
  wrap.append(titleElement, list);
  return wrap;
}

function renderHookInspector(hook) {
  hook.on = hook.on || { state: state.model.states[0]?.name || "", event: "changed" };
  elements.inspectorTitle.textContent = `Hook: ${hook.name}`;
  elements.inspectorBody.innerHTML = "";
  elements.inspectorBody.appendChild(input("Name", hook.name, (value) => {
    hook.name = value;
    rerenderAll();
  }));
  elements.inspectorBody.appendChild(select("On State.changed", hook.on.state, stateNames(), (value) => {
    hook.on.state = value;
    hook.on.event = "changed";
    rerenderAll();
  }));
  elements.inspectorBody.appendChild(checkboxList("Reads", hook.reads, (next) => {
    hook.reads = next;
    rerenderAll();
  }));
  elements.inspectorBody.appendChild(checkboxList("Writes", hook.writes, (next) => {
    hook.writes = next;
    rerenderAll();
  }));
}

function constructorNamesForState(stateName) {
  const item = state.model.states.find((candidate) => candidate.name === stateName);
  return item ? item.constructors.map((constructor) => constructor.name) : [];
}

function renderInstanceInspector(instance) {
  elements.inspectorTitle.textContent = `Instance: ${instance.name}`;
  elements.inspectorBody.innerHTML = "";
  elements.inspectorBody.appendChild(input("Name", instance.name, (value) => {
    instance.name = value;
    rerenderAll();
  }));
  elements.inspectorBody.appendChild(select("State", instance.state, stateNames(), (value) => {
    instance.state = value;
    delete instance.constructor;
    rerenderAll();
  }));
  const constructorOptions = ["", ...constructorNamesForState(instance.state)];
  elements.inspectorBody.appendChild(select("Constructor", instance.constructor || "", constructorOptions, (value) => {
    if (value === "") {
      delete instance.constructor;
    } else {
      instance.constructor = value;
    }
    rerenderAll();
  }));
}

function renderInspector() {
  const itemState = selectedState();
  if (itemState) {
    renderStateInspector(itemState);
    return;
  }
  const hook = selectedHook();
  if (hook) {
    renderHookInspector(hook);
    return;
  }
  const instance = selectedInstance();
  if (instance) {
    renderInstanceInspector(instance);
    return;
  }
  elements.inspectorTitle.textContent = "Nothing selected";
  elements.inspectorBody.innerHTML = "<p class=\"status\">Select a state, instance, or hook node to edit it.</p>";
}

function selectItem(type, index) {
  state.selected = { type, index };
  renderGraph();
  renderInspector();
}

async function loadProject() {
  const [payload, standardPayload] = await Promise.all([
    requestJson("/api/project"),
    requestJson("/api/standard-states"),
  ]);
  state.standardStates = standardPayload.standardStates || [];
  elements.standardStateSelect.innerHTML = "";
  for (const preset of state.standardStates) {
    const option = document.createElement("option");
    option.value = preset.id;
    option.textContent = preset.name;
    elements.standardStateSelect.appendChild(option);
  }
  state.project = payload.project;
  elements.projectPath.textContent = payload.project;
  elements.schemaSelect.innerHTML = "";
  for (const file of payload.files) {
    const option = document.createElement("option");
    option.value = file;
    option.textContent = file;
    elements.schemaSelect.appendChild(option);
  }
  if (payload.files.length > 0) {
    state.file = payload.files[0];
    elements.schemaSelect.value = state.file;
    await loadSchema();
  } else {
    setStatus("No JSON schema files found", true);
  }
}

async function loadSchema() {
  state.file = elements.schemaSelect.value;
  const payload = await requestJson(`/api/schema?file=${encodeURIComponent(state.file)}`);
  state.model = payload.model;
  state.source = payload.source;
  state.selected = null;
  elements.sourcePreview.value = state.source;
  setStatus("Loaded");
  renderGraph();
  renderInspector();
}

async function saveSchema() {
  const payload = await requestJson(`/api/schema?file=${encodeURIComponent(state.file)}`, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify({ source: renderLocalSource() }),
  });
  state.source = payload.source;
  elements.sourcePreview.value = state.source;
  setStatus("Saved");
}

async function generateSchema() {
  await saveSchema();
  const payload = await requestJson(`/api/generate?file=${encodeURIComponent(state.file)}`, { method: "POST" });
  setStatus(`Generated ${payload.files.join(", ")}`);
}

elements.schemaSelect.addEventListener("change", () => loadSchema().catch((error) => setStatus(error.message, true)));
elements.reloadButton.addEventListener("click", () => loadSchema().catch((error) => setStatus(error.message, true)));
elements.saveButton.addEventListener("click", () => saveSchema().catch((error) => setStatus(error.message, true)));
elements.generateButton.addEventListener("click", () => generateSchema().catch((error) => setStatus(error.message, true)));
elements.autoLayoutButton.addEventListener("click", () => runAutoLayout());
elements.addStateButton.addEventListener("click", () => {
  const name = uniqueName("State", stateNames());
  state.model.states.push({ name, fields: [{ name: "field1", type: "i32", default: 0 }], constructors: [] });
  selectItem("state", state.model.states.length - 1);
  rerenderAll();
});
elements.addStandardStateButton.addEventListener("click", () => {
  if (state.standardStates.length === 0) {
    setStatus("No standard states available", true);
    return;
  }
  const preset = state.standardStates.find((item) => item.id === elements.standardStateSelect.value);
  if (!preset) {
    setStatus("Invalid standard state selection", true);
    return;
  }
  if (!state.model.states.some((item) => item.name === preset.state.name)) {
    state.model.states.push(JSON.parse(JSON.stringify(preset.state)));
  }
  if (preset.instance && !state.model.instances.some((item) => item.name === preset.instance.name)) {
    state.model.instances.push(JSON.parse(JSON.stringify(preset.instance)));
  }
  state.selected = null;
  rerenderAll();
});
elements.addInstanceButton.addEventListener("click", () => {
  if (state.model.states.length === 0) {
    setStatus("Add a state before adding an instance", true);
    return;
  }
  const name = uniqueName("instance", state.model.instances.map((instance) => instance.name));
  state.model.instances.push({ name, state: state.model.states[0].name });
  selectItem("instance", state.model.instances.length - 1);
  rerenderAll();
});
elements.addHookButton.addEventListener("click", () => {
  const name = uniqueName("Hook", state.model.hooks.map((hook) => hook.name));
  state.model.hooks.push({ name, on: { state: state.model.states[0]?.name || "", event: "changed" }, reads: [], writes: [] });
  selectItem("hook", state.model.hooks.length - 1);
  rerenderAll();
});
elements.deleteButton.addEventListener("click", () => {
  if (!state.selected) {
    return;
  }
  if (state.selected.type === "state") {
    const removed = state.model.states.splice(state.selected.index, 1)[0];
    state.model.instances = state.model.instances.filter((instance) => instance.state !== removed.name);
    for (const hook of state.model.hooks) {
      hook.on = hook.on || {};
      if (hook.on.state === removed.name) {
        hook.on.state = state.model.states[0]?.name || "";
        hook.on.event = "changed";
      }
      hook.reads = hook.reads.filter((name) => name !== removed.name);
      hook.writes = hook.writes.filter((name) => name !== removed.name);
    }
  } else if (state.selected.type === "instance") {
    state.model.instances.splice(state.selected.index, 1);
  } else {
    state.model.hooks.splice(state.selected.index, 1);
  }
  state.selected = null;
  rerenderAll();
});
elements.sourcePreview.addEventListener("change", async () => {
  try {
    const payload = await requestJson("/api/render", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ source: elements.sourcePreview.value }),
    });
    state.model = payload.model;
    state.source = payload.source;
    state.selected = null;
    setStatus("Preview parsed");
    renderGraph();
    renderInspector();
  } catch (error) {
    setStatus(error.message, true);
  }
});

loadProject().catch((error) => setStatus(error.message, true));
