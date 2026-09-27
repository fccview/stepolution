var catalog = require("./catalog.json");

function select(messageKey, label, defaultValue, options) {
  return {
    type: "select",
    messageKey: messageKey,
    label: label,
    defaultValue: defaultValue,
    options: options.map(function (option, index) {
      return { label: option, value: String(index) };
    }),
  };
}

function withRandom(item) {
  item.options.unshift({ label: "Random (new one every day)", value: "255" });
  return item;
}

module.exports = [
  { type: "heading", defaultValue: "Stepolution" },
  {
    type: "section",
    items: [
      { type: "heading", defaultValue: "Trainer" },
      select("Trainer", "Character", "0", ["Red", "Leaf"]),
      withRandom(select("Starter", "Pokemon", "255", catalog.pokemon)),
    ],
  },
  {
    type: "section",
    items: [
      { type: "heading", defaultValue: "World" },
      withRandom(select("Scene", "Location", "255", catalog.scenes)),
      select("Frame", "Window frame", "0", catalog.frames),
      select("Daylight", "Time of day", "0", [
        "Follow the clock",
        "Always day",
        "Always dusk",
        "Always night",
      ]),
      {
        type: "select",
        messageKey: "Zoom",
        label: "Zoom",
        defaultValue: "1",
        options: [
          { label: "Classic (1x)", value: "1" },
          { label: "Close up (2x)", value: "2" },
        ],
      },
      {
        type: "select",
        messageKey: "WanderSeconds",
        label: "Wander every",
        defaultValue: "60",
        options: [
          { label: "Never", value: "0" },
          { label: "5 seconds", value: "5" },
          { label: "10 seconds", value: "10" },
          { label: "20 seconds", value: "20" },
          { label: "1 minute", value: "60" },
          { label: "5 minutes", value: "300" },
        ],
      },
      {
        type: "toggle",
        messageKey: "IdleAnimation",
        label: "Pokemon steps in place",
        description: "Keeps your Pokemon moving while it stands still. Uses a bit more battery.",
        defaultValue: false,
      },
      {
        type: "toggle",
        messageKey: "ActiveOnly",
        label: "Only animate when the light is on",
        description: "Walks, greetings and surprises wait until the backlight turns on. Saves battery.",
        defaultValue: true,
      },
    ],
  },
  {
    type: "section",
    items: [
      { type: "heading", defaultValue: "Evolution" },
      {
        type: "input",
        messageKey: "StepGoal",
        label: "Daily step goal",
        defaultValue: "8000",
        description: "Evolves at 50% and 100% of the goal.",
        attributes: { type: "number", min: 1000, max: 100000 },
      },
      {
        type: "toggle",
        messageKey: "Demo",
        label: "Demo mode",
        description:
          "Fakes steps so you can watch it evolve in about a minute.",
        defaultValue: false,
      },
    ],
  },
  { type: "submit", defaultValue: "Save" },
];
