const assert = require('assert');
const fs = require('fs');
const path = require('path');
const vscode = require('vscode');

async function until(condition, description, seconds = 60) {
    const deadline = Date.now() + seconds * 1000;
    while (Date.now() < deadline) {
        const result = condition();
        if (result) return result;
        await new Promise(resolve => setTimeout(resolve, 100));
    }
    throw new Error(`Timed out: ${description}`);
}

exports.run = async function () {
    const folder = vscode.workspace.workspaceFolders[0];
    const root = folder.uri.fsPath;
    const tasks = [];
    const messages = [];
    const sessions = [];
    const report = [];
    let stage = 'starting';
    const save = () => fs.writeFileSync(process.env.PRYSMA_VSCODE_REPORT, JSON.stringify({ report, tasks, stage }, null, 2));
    save();
    const subscriptions = [
        vscode.tasks.onDidStartTask(event => { tasks.push({ name: event.execution.task.name, state: 'start' }); save(); }),
        vscode.tasks.onDidEndTaskProcess(event => { tasks.push({ name: event.execution.task.name, state: 'end', code: event.exitCode }); save(); }),
        vscode.debug.onDidStartDebugSession(session => sessions.push(session)),
        vscode.debug.registerDebugAdapterTrackerFactory('*', {
            createDebugAdapterTracker(session) {
                return { onDidSendMessage(message) { messages.push({ session, message }); } };
            }
        })
    ];

    async function debug(name, relative, instruction, expectedVariable, expectedValue) {
        stage = name;
        save();
        const source = path.join(root, relative);
        const document = await vscode.workspace.openTextDocument(source);
        await vscode.window.showTextDocument(document);
        const line = document.getText().split(/\r?\n/).findIndex(text => text.includes(instruction));
        assert(line >= 0, `Missing breakpoint instruction: ${instruction}`);
        const breakpoint = new vscode.SourceBreakpoint(new vscode.Location(document.uri, new vscode.Position(line, 0)));
        vscode.debug.addBreakpoints([breakpoint]);
        messages.length = 0;
        const before = sessions.length;
        let launchResult;
        let launchError;
        vscode.debug.startDebugging(folder, name).then(result => { launchResult = result; }, error => { launchError = error; });
        await until(() => launchResult !== undefined || launchError, `${name} launch`, 600);
        if (launchError) throw launchError;
        assert(launchResult, `Cannot start ${name}`);
        const session = await until(() => sessions[before], `${name} session`);
        const stop = await until(() => messages.find(item => item.session.id === session.id && item.message.event === 'stopped'), `${name} breakpoint`, 180);
        assert.strictEqual(stop.message.body.reason, 'breakpoint');
        const threadId = stop.message.body.threadId;
        const stack = await session.customRequest('stackTrace', { threadId });
        const frame = stack.stackFrames[0];
        assert.strictEqual(path.resolve(frame.source.path), path.resolve(source));
        assert.strictEqual(frame.line, line + 1);
        const value = await session.customRequest('evaluate', { frameId: frame.id, expression: expectedVariable, context: 'watch' });
        assert.strictEqual(value.result, expectedValue);
        vscode.debug.removeBreakpoints([breakpoint]);
        let terminated = false;
        const beforeStop = tasks.length;
        const listener = vscode.debug.onDidTerminateDebugSession(event => { if (event.id === session.id) terminated = true; });
        await vscode.debug.stopDebugging(session);
        await until(() => terminated, `${name} termination`);
        listener.dispose();
        const stopTaskName = name.startsWith('Python') ? 'Stop Python Debug Server (Docker)' : 'Stop LLDB Server (Docker)';
        const stopTask = await until(() => tasks.slice(beforeStop).find(task => task.name === stopTaskName && task.state === 'end'), `${stopTaskName} completion`);
        assert.strictEqual(stopTask.code, 0, `${stopTaskName} failed with exit code ${stopTask.code}`);
        report.push({ name, status: 'passed', source: frame.source.path, line: frame.line });
        save();
    }

    try {
        await vscode.workspace.getConfiguration('debug').update('onTaskErrors', 'abort', vscode.ConfigurationTarget.Workspace);
        await debug('Python: Debug in Docker', 'tests/environnement_dev_tests/fixtures/debug_target.py',
                    'result = compute_double(int_var)', 'int_var', '42');
        await debug('C++: Debug Prysma in Docker (LLDB)', 'src/main.cpp', 'if (argc < 2)', 'argc', '2');
        const build = tasks.findIndex(task => task.name === 'Build' && task.state === 'end' && task.code === 0);
        const launch = tasks.findIndex(task => task.name === 'Start LLDB Server (Docker)' && task.state === 'start');
        assert(build >= 0 && launch > build, 'Build must finish before starting LLDB');

        // Keep the successful binary, then deliberately break the source in this temporary copy.
        fs.appendFileSync(path.join(root, 'src/main.cpp'), '\n#error deliberate integration test failure\n');
        stage = 'failed build';
        save();
        const before = sessions.length;
        let failedLaunchResult;
        vscode.debug.startDebugging(folder, 'C++: Debug Prysma in Docker (LLDB)').then(result => { failedLaunchResult = result; });
        await until(() => failedLaunchResult !== undefined, 'failed build aborts launch', 120);
        assert.strictEqual(failedLaunchResult, false);
        assert.strictEqual(sessions.length, before, 'Failed build launched a stale binary');
        assert(tasks.some(task => task.name === 'Build' && task.state === 'end' && task.code !== 0));
        report.push({ name: 'Failed build blocks stale binary', status: 'passed' });
        stage = 'complete';
    } catch (error) {
        report.push({ name: stage, status: 'failed', error: String(error.stack || error) });
        throw error;
    } finally {
        save();
        if (vscode.debug.activeDebugSession) await vscode.debug.stopDebugging(vscode.debug.activeDebugSession);
        for (const task of vscode.tasks.taskExecutions) task.terminate();
        subscriptions.forEach(subscription => subscription.dispose());
        save();
    }
};
