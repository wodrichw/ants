const execa = require('execa');

module.exports = async (pluginConfig, context) => {
  const { cmd } = pluginConfig;
  context.logger.log(`Running command: ${cmd}`);
  await execa.command(cmd, { stdio: 'inherit' });
};
