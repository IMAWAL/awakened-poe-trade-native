import assert from 'node:assert/strict'
import { readFileSync } from 'node:fs'
import { test } from 'node:test'
import { runInNewContext } from 'node:vm'
import ts from 'typescript'

// Exercise passive input without requiring the game to give up keyboard focus.
function captureInput (kind, { form = true, textarea = false } = {}) {
  const events = []
  const messages = []
  const callbacks = new Map()
  let pointerdown

  class Input {
    value = ''
    type = kind === 'number' ? 'number' : 'text'
    readOnly = false
    disabled = false
    selectionStart = 0
    selectionEnd = 0
    form = form ? { requestSubmit: () => events.push(['submit', this.value]) } : null

    dispatchEvent (event) {
      events.push([event.type, this.value])
    }

    blur () {}

    setRangeText (text, start, end) {
      this.value = this.value.slice(0, start) + text + this.value.slice(end)
      this.selectionStart = this.selectionEnd = start + text.length
    }
  }
  class Textarea extends Input {}

  const Host = {
    onEvent (name, callback) {
      callbacks.set(name, callback)
      return { abort () {} }
    },
    sendEvent (message) { messages.push(message) }
  }
  const exports = {}
  const source = readFileSync(new URL(`../src/web/native-${kind}-input.ts`, import.meta.url), 'utf8')
  runInNewContext(ts.transpileModule(source, {
    compilerOptions: { module: ts.ModuleKind.CommonJS, target: ts.ScriptTarget.ES2022 }
  }).outputText, {
    exports,
    require (name) {
      assert.equal(name, '@/web/background/IPC')
      return { Host }
    },
    HTMLInputElement: Input,
    HTMLTextAreaElement: Textarea,
    Event,
    document: {
      addEventListener (name, callback) {
        assert.equal(name, 'pointerdown')
        pointerdown = callback
      }
    },
    window: { addEventListener () {} }
  })
  exports[kind === 'number' ? 'installNativeNumberInputCapture' : 'installNativeTextInputCapture']()
  const input = textarea ? new Textarea() : new Input()
  return {
    input,
    events,
    messages,
    click () { pointerdown({ target: input }) },
    key (key, token = messages[0].payload.token) {
      callbacks.get(`MAIN->CLIENT::${kind === 'number' ? 'number' : 'text'}-captured`)({ key, token })
    }
  }
}

test('passive numeric Enter submits the edited filter once and finishes capture', () => {
  const capture = captureInput('number')
  capture.input.value = '10'
  capture.click()
  capture.key('4')
  capture.key('2')
  capture.key('Enter')
  capture.key('Enter')
  assert.deepEqual(capture.events, [
    ['input', '4'], ['input', '42'], ['change', '42'], ['submit', '42']
  ])
})

test('numeric Escape restores the original filter without submitting', () => {
  const capture = captureInput('number')
  capture.input.value = '10'
  capture.click()
  capture.key('4')
  capture.key('Escape')
  capture.key('Enter')
  assert.equal(capture.input.value, '10')
  assert.deepEqual(capture.events, [['input', '4'], ['input', '10']])
})

test('passive text Enter submits and cancels the native capture', () => {
  const capture = captureInput('text')
  capture.click()
  capture.key('a')
  capture.key('Enter')
  capture.key('Enter')
  assert.deepEqual(capture.events, [['input', 'a'], ['change', 'a'], ['submit', 'a']])
  assert.equal(capture.messages.at(-1).name, 'CLIENT->MAIN::cancel-text-capture')
})

test('textarea Enter inserts a newline without submitting', () => {
  const capture = captureInput('text', { textarea: true })
  capture.click()
  capture.key('a')
  capture.key('Enter')
  assert.equal(capture.input.value, 'a\n')
  assert.deepEqual(capture.events, [['input', 'a'], ['input', 'a\n']])
})

for (const kind of ['number', 'text']) {
  test(`${kind} Enter also works outside a form`, () => {
    const capture = captureInput(kind, { form: false })
    capture.click()
    capture.key('4')
    capture.key('Enter')
    assert.deepEqual(capture.events, [['input', '4'], ['change', '4']])
  })

  test(`${kind} capture ignores events for other controls`, () => {
    const capture = captureInput(kind)
    capture.click()
    capture.key('4', 'another-control')
    capture.key('Enter', 'another-control')
    assert.deepEqual(capture.events, [])
  })
}
