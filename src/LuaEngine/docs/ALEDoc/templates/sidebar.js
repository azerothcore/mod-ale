document.write(`
<div class="border-b-2 border-line-strong">
  <div class="flex items-center justify-between px-4 py-2.5 text-[11px] font-bold uppercase tracking-wider text-body-dim">
    {{ current_class.name }}
    <span class="rounded-full border border-line bg-ink-raised px-1.5 font-semibold normal-case tracking-normal">{{ current_class.methods | length }}</span>
  </div>
  <div class="px-2 pb-2">
{%- for method in current_class.methods %}
    <a id="{{ current_class.name + ':' + method.name }}" class="block truncate rounded px-2.5 py-1 text-sm hover:bg-ink-raised hover:text-body-bright" href="{{ root(current_class.name + '/' + method.name + '.html') }}">{{ method.name }}</a>
{%- endfor %}
  </div>
</div>
`);
